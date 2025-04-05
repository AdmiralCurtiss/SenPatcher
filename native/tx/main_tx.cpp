#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

#include "util/file.h"
#include "util/hash/crc32.h"
#include "util/ini.h"
#include "util/logger.h"

#include "modload/loaded_mods.h"
#include "modload/loaded_pka.h"
#include "sen/pka.h"
#include "sen/pkg.h"
#include "sen/pkg_extract.h"
#include "tx/exe_patch.h"
#include "tx/file_fixes.h"
#include "tx/inject_modloader.h"
#include "util/text.h"

#include "senpatcher_version.h"

extern "C" {
__declspec(dllexport) char SenPatcherHook[] = "TX";
__declspec(dllexport) char SenPatcherVersion[] = SENPATCHER_VERSION;
}

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// #define DEBUG_OUTPUT_FOR_INJECTED_FILES

using SenLib::TX::GameVersion;

using PDirectInput8Create = HRESULT(
    __stdcall*)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, void* punkOuter);
static PDirectInput8Create LoadForwarderAddress(HyoutaUtils::Logger& logger) {
    constexpr int total = 5000;
    WCHAR tmp[total];
    UINT count = GetSystemDirectoryW(tmp, sizeof(tmp) / sizeof(WCHAR));
    if (count == 0 || count > total - 16) {
        logger.Log("Failed constructing path for system dinput8.dll.\n");
        return nullptr;
    }
    std::memcpy(tmp + count, L"\\dinput8.dll\0", sizeof(L"\\dinput8.dll\0"));

    HMODULE dll = ::LoadLibraryW(tmp);
    if (!dll) {
        logger.Log("Failed loading system dinput8.dll.\n");
        return nullptr;
    }
    void* addr = ::GetProcAddress(dll, "DirectInput8Create");
    if (!addr) {
        logger.Log("Failed finding system DirectInput8Create.\n");
    } else {
        logger.Log("Found system DirectInput8Create at ").LogPtr(addr).Log(".\n");
    }
    return (PDirectInput8Create)addr;
}

static void Align16CodePage(HyoutaUtils::Logger& logger, char*& new_page) {
    logger.Log("Aligning ").LogPtr(new_page).Log(" to 16 bytes.\n");
    char* p = new_page;
    while ((std::bit_cast<uint32_t>(p) & 0xf) != 0) {
        *p++ = static_cast<char>(0xcc);
    }
    new_page = p;
}

static void LogMemoryMap(HyoutaUtils::Logger& logger) {
    MEMORY_BASIC_INFORMATION info;
    std::memset(&info, 0, sizeof(info));
    size_t address = 0;
    while (true) {
        if (VirtualQuery(reinterpret_cast<void*>(address), &info, sizeof(info)) == 0) {
            logger.Log("Querying address ").LogHex(address).Log(" failed.\n");
            break;
        }
        if (info.RegionSize == 0) {
            logger.Log("Querying address ").LogHex(address).Log(" returned zero-sized region.\n");
            break;
        }

        logger.Log("Address ")
            .LogHex(address)
            .Log(", allocation at ")
            .LogPtr(info.AllocationBase)
            .Log(", base ptr ")
            .LogPtr(info.BaseAddress)
            .Log(", size ")
            .LogHex(info.RegionSize)
            .Log(", protection ")
            .LogHex(info.Protect)
            .Log(".\n");

        if (address >= (size_t(0) - info.RegionSize)) {
            // would wrap around
            break;
        }

        address += info.RegionSize;
    }
}

static std::optional<GameVersion> FindImageBase(HyoutaUtils::Logger& logger, void** code) {
    MEMORY_BASIC_INFORMATION info;
    std::memset(&info, 0, sizeof(info));
    *code = nullptr;
    size_t address = 0;
    while (true) {
        if (VirtualQuery(reinterpret_cast<void*>(address), &info, sizeof(info)) == 0) {
            break;
        }
        if (info.RegionSize == 0) {
            break;
        }

        if (info.State == MEM_COMMIT && info.Type == MEM_IMAGE) {
            if (info.RegionSize == 0x576000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x44a280 - 0x401000), 0x1d);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x077c1d1a) {
                    logger.Log("Appears to be the GOG version.\n");
                    *code = info.BaseAddress;
                    return GameVersion::Gog;
                }
            } else if (info.RegionSize == 0x577000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x44b930 - 0x401000), 0x1d);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x077c1d1a) {
                    logger.Log("Appears to be the Steam version.\n");
                    *code = info.BaseAddress;
                    return GameVersion::Steam;
                }
                if (crc == 0xff6f70fd) {
                    logger.Log("Appears to be the Steam version (encryped).\n");
                    *code = info.BaseAddress;
                    return GameVersion::Steam_Encrypted;
                }
            }

            // logger.Log("Memory is: ");
            // for (int i = 0; i < info.RegionSize; ++i) {
            //  logger.Log(" ").LogHex(*(reinterpret_cast<unsigned char*>(info.BaseAddress) + i));
            // }
            // logger.Log("\n");
        }

        if (address >= (size_t(0) - info.RegionSize)) {
            // would wrap around
            break;
        }

        address += info.RegionSize;
    }

    return std::nullopt;
}

struct FFile {
    void* vfuncptr;
    uint32_t Filesize;
    void* NativeFileHandle;
    void* ExternalClassPointer;
    void* MemoryPointer;

    // if nonzero it will be free'd when the struct is released
    void* MemoryPointerForFreeing;

    uint32_t MemoryPosition;
};
static_assert(offsetof(FFile, Filesize) == 0x4);
static_assert(offsetof(FFile, NativeFileHandle) == 0x8);
static_assert(offsetof(FFile, ExternalClassPointer) == 0xc);
static_assert(offsetof(FFile, MemoryPointer) == 0x10);
static_assert(offsetof(FFile, MemoryPointerForFreeing) == 0x14);
static_assert(offsetof(FFile, MemoryPosition) == 0x18);

using PTrackedMalloc = void*(
    __cdecl*)(uint32_t size, uint32_t alignment, const char* file, uint32_t line, uint32_t unknown);
using PTrackedFree = void(__cdecl*)(void* memory);
using PPrFileMalloc = void*(__fastcall*)(uint32_t size);
using PPPrFileFree = void(__cdecl**)(void* memory);

static PTrackedMalloc s_TrackedMalloc = nullptr;
static PTrackedFree s_TrackedFree = nullptr;
static PPrFileMalloc s_PrFileBufferMalloc = nullptr;
static PPPrFileFree s_Ptr_PrFileBufferFree = nullptr;

static SenLib::ModLoad::LoadedP3AData s_LoadedVanillaP3As{};
static constexpr std::array<std::string_view, 5> s_VanillaP3ANames{
    {"misc_jp.p3a", "misc.p3a", "bgm.p3a", "se.p3a", "voice.p3a"}};

static SenLib::ModLoad::LoadedModsData s_LoadedModsData{};

static SenLib::ModLoad::LoadedPkaData s_LoadedPkaData{};
static constexpr std::array<std::string_view, 2> s_PkaGroupPrefixes{{"", ""}};
static constexpr std::array<std::string_view, 2> s_PkaNames{{"assets_jp", "assets"}};

static bool s_JapaneseLanguage = false;

// ignore any path that doesn't begin with the 'data' directory
static bool IsValidReroutablePath(const char* path) {
    // TX doesn't prefix its paths with 'data', which makes this a bit annoying to check.
    // We'll just assume that any relative path is reroutable...
    if (!path || path[0] == '\0' || path[0] == '\\' || path[0] == '//' || path[1] == ':') {
        return false;
    }
    return true;
}

// returns:
// 0 if we should report that the file open failed
// 1 if we should report that the file open succeeded
// -1 if the original function should continue running
static int32_t OpenModFile(FFile* ffile, const char* path) {
    if (!IsValidReroutablePath(path)) {
        return -1;
    }

    if (s_LoadedModsData.CheckDevFolderForAssets) {
        const size_t path_len = strlen(path);
        std::string tmp;
        tmp.reserve(4 + path_len);
        tmp.append("dev/");
        tmp.append(path);

        HyoutaUtils::IO::File file(std::string_view(tmp), HyoutaUtils::IO::OpenMode::Read);
        if (file.IsOpen()) {
            auto length = file.GetLength();
            if (!length) {
                ffile->NativeFileHandle = INVALID_HANDLE_VALUE;
                return 0;
            }
            ffile->NativeFileHandle = file.ReleaseHandle();
            ffile->Filesize = static_cast<uint32_t>(*length);
            return 1;
        }
    }

    std::array<char, 0x100> filteredPath;
    if (!SenLib::ModLoad::FilterGamePath(filteredPath.data(), path, filteredPath.size())) {
        return -1;
    }

    const SenLib::ModLoad::P3AFileRef* refptr =
        FindP3AFileRef(s_LoadedModsData.LoadedP3As, filteredPath);
    if (refptr == nullptr) {
        refptr = FindP3AFileRef(s_LoadedVanillaP3As, filteredPath);
    }
    if (refptr != nullptr) {
        void* memory = nullptr;
        uint64_t filesize = 0;
        if (!SenLib::ModLoad::ExtractP3AFileToMemory(
                *refptr,
                refptr->ArchiveData->Flags,
                0x8000'0000,
                memory,
                filesize,
                [](size_t length) { return s_TrackedMalloc(length, 8, nullptr, 0, 0); },
                [](void* memory) { s_TrackedFree(memory); })) {
            ffile->NativeFileHandle = INVALID_HANDLE_VALUE;
            return 0;
        }
        ffile->Filesize = static_cast<uint32_t>(filesize);
        ffile->MemoryPointer = memory;
        ffile->MemoryPointerForFreeing = memory;
        ffile->MemoryPosition = 0;
        return 1;
    }

    const auto& pkaData = s_LoadedPkaData;
    const auto checkPka = [&](SenLib::PkaPkgToHashData* pkgs,
                              size_t pkgCount,
                              SenLib::PkaFileHashData* pkgFiles,
                              size_t pkgFilesCount,
                              const char* pkgPrefix,
                              size_t pkgPrefixLength) -> int32_t {
        if (pkgCount > 0 && memcmp(pkgPrefix, filteredPath.data(), pkgPrefixLength) == 0) {
            // check for data in the PKA
            const size_t start = pkgPrefixLength;
            assert(filteredPath.size() - start >= pkgs[0].PkgName.size());
            const SenLib::PkaPkgToHashData* pkaPkg =
                SenLib::FindPkgInPkaByName(pkgs, pkgCount, &filteredPath[start]);
            if (pkaPkg) {
                // check bounds
                // 0x120'0000 is an arbitrary limit; it would result in an allocation of near 2 GB
                // just for the header which is bound to fail in 32-bit address space anyway
                if (pkaPkg->FileCount > 0x120'0000 || pkaPkg->FileOffset > pkgFilesCount
                    || pkaPkg->FileCount > pkgFilesCount - pkaPkg->FileOffset) {
                    ffile->NativeFileHandle = INVALID_HANDLE_VALUE;
                    return 0;
                }

                size_t length = 8 + (pkaPkg->FileCount * (0x50 + 0x20));
                void* memory = s_TrackedMalloc(length, 8, nullptr, 0, 0);
                if (!memory) {
                    ffile->NativeFileHandle = INVALID_HANDLE_VALUE;
                    return 0;
                }

                // build fake pkg
                if (!SenLib::ModLoad::BuildFakePkaPkg((char*)memory,
                                                      pkaPkg,
                                                      pkgFiles,
                                                      pkaData.Hashes.Files.get(),
                                                      pkaData.Hashes.FilesCount)) {
                    s_TrackedFree(memory);
                    ffile->NativeFileHandle = INVALID_HANDLE_VALUE;
                    return 0;
                }

                ffile->Filesize = static_cast<uint32_t>(length);
                ffile->MemoryPointer = memory;
                ffile->MemoryPointerForFreeing = memory;
                ffile->MemoryPosition = 0;
                return 1;
            }
        }
        return -1;
    };

    const auto& pkaPrefixes = s_PkaGroupPrefixes;
    for (size_t i = 0; i < pkaPrefixes.size(); ++i) {
        int32_t pkaCheckResult = checkPka(pkaData.Groups[i].Pkgs.get(),
                                          pkaData.Groups[i].PkgCount,
                                          pkaData.Groups[i].PkgFiles.get(),
                                          pkaData.Groups[i].PkgFileCount,
                                          pkaPrefixes[i].data(),
                                          pkaPrefixes[i].size());
        if (pkaCheckResult >= 0) {
            return pkaCheckResult;
        }
    }

    return -1;
}

// returns:
// 0 if we should report that the file does not exist
// 1 if we should report that the file does exist (in only this case *out_filesize will be set)
// -1 if the original function should continue running
static int32_t GetFilesizeOfModFile(const char* path, uint32_t* out_filesize) {
    if (!IsValidReroutablePath(path)) {
        return -1;
    }

    if (s_LoadedModsData.CheckDevFolderForAssets) {
        const size_t path_len = strlen(path);
        std::string tmp;
        tmp.reserve(4 + path_len);
        tmp.append("dev/");
        tmp.append(path);

        HyoutaUtils::IO::File file(std::string_view(tmp), HyoutaUtils::IO::OpenMode::Read);
        if (file.IsOpen()) {
            auto length = file.GetLength();
            if (!length) {
                return 0;
            }
            if (out_filesize) {
                *out_filesize = static_cast<uint32_t>(*length);
            }
            return 1;
        }
    }

    std::array<char, 0x100> filteredPath;
    if (!SenLib::ModLoad::FilterGamePath(filteredPath.data(), path, filteredPath.size())) {
        return -1;
    }

    const SenLib::ModLoad::P3AFileRef* refptr =
        FindP3AFileRef(s_LoadedModsData.LoadedP3As, filteredPath);
    if (refptr == nullptr) {
        refptr = FindP3AFileRef(s_LoadedVanillaP3As, filteredPath);
    }
    if (refptr != nullptr) {
        const SenLib::ModLoad::P3AFileRef& ref = *refptr;
        const SenPatcher::P3AFileInfo& fi = *ref.FileInfo;
        if (out_filesize) {
            *out_filesize = static_cast<uint32_t>(fi.UncompressedSize);
        }
        return 1;
    }

    const auto& pkaData = s_LoadedPkaData;
    const auto& pkaPrefixes = s_PkaGroupPrefixes;
    for (size_t i = 0; i < pkaPrefixes.size(); ++i) {
        int32_t pkaCheckResult = SenLib::ModLoad::GetPkaPkgFilesize(pkaData.Groups[i].Pkgs.get(),
                                                                    pkaData.Groups[i].PkgCount,
                                                                    pkaPrefixes[i].data(),
                                                                    pkaPrefixes[i].size(),
                                                                    filteredPath,
                                                                    path,
                                                                    out_filesize,
                                                                    false);
        if (pkaCheckResult >= 0) {
            return pkaCheckResult;
        }
    }

    return -1;
}

// returns:
// 0 if we should report that the file open failed
// 1 if we should report that the file open succeeded
// -1 if the original function should continue running
static int32_t OpenPrModFile(SenLib::TX::PrFileHelperStruct* prFile, const char* path) {
    if (!IsValidReroutablePath(path)) {
        return -1;
    }

    if (s_LoadedModsData.CheckDevFolderForAssets) {
        const size_t path_len = strlen(path);
        std::string tmp;
        tmp.reserve(4 + path_len);
        tmp.append("dev/");
        tmp.append(path);

        HyoutaUtils::IO::File file(std::string_view(tmp), HyoutaUtils::IO::OpenMode::Read);
        if (file.IsOpen()) {
            auto length = file.GetLength();
            if (!length) {
                return 0;
            }
            const uint32_t bufferSize = static_cast<uint32_t>(*length);
            void* memory = s_PrFileBufferMalloc(bufferSize);
            if (!memory) {
                return 0;
            }
            if (file.Read(memory, bufferSize) != bufferSize) {
                (*s_Ptr_PrFileBufferFree)(memory);
                return 0;
            }

            prFile->BufferSize = bufferSize;
            prFile->DataBuffer = memory;
            return 1;
        }
    }

    std::array<char, 0x100> filteredPath;
    if (!SenLib::ModLoad::FilterGamePath(filteredPath.data(), path, filteredPath.size())) {
        return -1;
    }

    const SenLib::ModLoad::P3AFileRef* refptr =
        FindP3AFileRef(s_LoadedModsData.LoadedP3As, filteredPath);
    if (refptr == nullptr) {
        refptr = FindP3AFileRef(s_LoadedVanillaP3As, filteredPath);
    }
    if (refptr != nullptr) {
        void* memory = nullptr;
        uint64_t filesize = 0;
        if (!SenLib::ModLoad::ExtractP3AFileToMemory(
                *refptr,
                refptr->ArchiveData->Flags,
                0x8000'0000,
                memory,
                filesize,
                [](size_t length) { return s_PrFileBufferMalloc(length); },
                [](void* memory) { (*s_Ptr_PrFileBufferFree)(memory); })) {
            return 0;
        }
        prFile->BufferSize = static_cast<uint32_t>(filesize);
        prFile->DataBuffer = memory;
        return 1;
    }

    const auto& pkaData = s_LoadedPkaData;
    const auto checkPka = [&](SenLib::PkaPkgToHashData* pkgs,
                              size_t pkgCount,
                              SenLib::PkaFileHashData* pkgFiles,
                              size_t pkgFilesCount,
                              const char* pkgPrefix,
                              size_t pkgPrefixLength) -> int32_t {
        if (pkgCount > 0 && memcmp(pkgPrefix, filteredPath.data(), pkgPrefixLength) == 0) {
            // check for data in the PKA
            const size_t start = pkgPrefixLength;
            assert(filteredPath.size() - start >= pkgs[0].PkgName.size());
            const SenLib::PkaPkgToHashData* pkaPkg =
                SenLib::FindPkgInPkaByName(pkgs, pkgCount, &filteredPath[start]);
            if (pkaPkg) {
                // check bounds
                // 0x120'0000 is an arbitrary limit; it would result in an allocation of near 2 GB
                // just for the header which is bound to fail in 32-bit address space anyway
                if (pkaPkg->FileCount > 0x120'0000 || pkaPkg->FileOffset > pkgFilesCount
                    || pkaPkg->FileCount > pkgFilesCount - pkaPkg->FileOffset) {
                    return 0;
                }

                size_t length = 8 + (pkaPkg->FileCount * (0x50 + 0x20));
                void* memory = s_PrFileBufferMalloc(length);
                if (!memory) {
                    return 0;
                }

                // build fake pkg
                if (!SenLib::ModLoad::BuildFakePkaPkg((char*)memory,
                                                      pkaPkg,
                                                      pkgFiles,
                                                      pkaData.Hashes.Files.get(),
                                                      pkaData.Hashes.FilesCount)) {
                    (*s_Ptr_PrFileBufferFree)(memory);
                    return 0;
                }

                prFile->BufferSize = static_cast<uint32_t>(length);
                prFile->DataBuffer = memory;
                return 1;
            }
        }
        return -1;
    };

    const auto& pkaPrefixes = s_PkaGroupPrefixes;
    for (size_t i = 0; i < pkaPrefixes.size(); ++i) {
        int32_t pkaCheckResult = checkPka(pkaData.Groups[i].Pkgs.get(),
                                          pkaData.Groups[i].PkgCount,
                                          pkaData.Groups[i].PkgFiles.get(),
                                          pkaData.Groups[i].PkgFileCount,
                                          pkaPrefixes[i].data(),
                                          pkaPrefixes[i].size());
        if (pkaCheckResult >= 0) {
            return pkaCheckResult;
        }
    }

    return -1;
}

static int32_t __fastcall FFileOpenForwarder(FFile* ffile, const char* path) {
#ifdef DEBUG_OUTPUT_FOR_INJECTED_FILES
    OutputDebugStringA("called FFileOpenForwarder(): ");
    OutputDebugStringA(path);
#endif
    const int32_t result = OpenModFile(ffile, path);
#ifdef DEBUG_OUTPUT_FOR_INJECTED_FILES
    if (result < 0) {
        OutputDebugStringA("  -> no modded file\n");
    } else if (result > 0) {
        OutputDebugStringA("  -> success\n");
    } else {
        OutputDebugStringA("  -> fail\n");
    }
#endif
    return result;
}

static int32_t __fastcall FFileGetFilesizeForwarder(const char* path, uint32_t* out_filesize) {
#ifdef DEBUG_OUTPUT_FOR_INJECTED_FILES
    OutputDebugStringA("called FFileGetFilesizeForwarder(): ");
    OutputDebugStringA(path);
#endif
    const int32_t result = GetFilesizeOfModFile(path, out_filesize);
#ifdef DEBUG_OUTPUT_FOR_INJECTED_FILES
    if (result < 0) {
        OutputDebugStringA("  -> no modded file\n");
    } else if (result > 0) {
        if (out_filesize) {
            OutputDebugStringA("  -> success, filesize = ");
            char buffer[32];
            sprintf(buffer, "%u", *out_filesize);
            OutputDebugStringA(buffer);
            OutputDebugStringA("\n");
        } else {
            OutputDebugStringA("  -> success\n");
        }
    } else {
        OutputDebugStringA("  -> fail\n");
    }
#endif
    return result;
}

static int32_t __fastcall PrFileOpenForwarder(SenLib::TX::PrFileHelperStruct* prFile,
                                              const char* path) {
#ifdef DEBUG_OUTPUT_FOR_INJECTED_FILES
    OutputDebugStringA("called PrFileOpenForwarder(): ");
    OutputDebugStringA(path);
#endif
    const int32_t result = OpenPrModFile(prFile, path);
#ifdef DEBUG_OUTPUT_FOR_INJECTED_FILES
    if (result < 0) {
        OutputDebugStringA("  -> no modded file\n");
    } else if (result > 0) {
        OutputDebugStringA("  -> success\n");
    } else {
        OutputDebugStringA("  -> fail\n");
    }
#endif
    return result;
}

struct PkgSingleFileHeader {
    std::array<char, 0x40> Filename;
    uint32_t UncompressedSize;
    uint32_t CompressedSize;
    uint32_t DataOffset;
    uint32_t Flags;
};
static_assert(offsetof(PkgSingleFileHeader, Filename) == 0);
static_assert(offsetof(PkgSingleFileHeader, UncompressedSize) == 0x40);
static_assert(offsetof(PkgSingleFileHeader, CompressedSize) == 0x44);
static_assert(offsetof(PkgSingleFileHeader, DataOffset) == 0x48);
static_assert(offsetof(PkgSingleFileHeader, Flags) == 0x4c);

static uint32_t __fastcall DecompressPkgForwarder(const char* compressedData,
                                                  char* decompressedData,
                                                  const PkgSingleFileHeader* pkgSingleFileHeader) {
    const uint32_t uncompressedSize = pkgSingleFileHeader->UncompressedSize;
    const uint32_t compressedSize = pkgSingleFileHeader->CompressedSize;
    const uint32_t flags = pkgSingleFileHeader->Flags;
    if ((flags & 0x80) && compressedSize == 0x20) {
        // this is a PKA hash, look up the data in the PKA
        auto& pkaData = s_LoadedPkaData;
        const SenLib::PkaHashToFileData* fileData = SenLib::FindFileInPkaByHash(
            pkaData.Hashes.Files.get(), pkaData.Hashes.FilesCount, compressedData);
        if (fileData && fileData->UncompressedSize == uncompressedSize) {
            std::unique_ptr<char[]> buffer = nullptr;
            const size_t index = static_cast<size_t>(static_cast<size_t>(fileData->Offset >> 48)
                                                     & static_cast<size_t>(0xffff));
            if (index < pkaData.HandleCount) {
                auto& pka = pkaData.Handles[index];
                std::lock_guard<std::recursive_mutex> lock(pka.Mutex);
                if (pka.Handle.SetPosition(fileData->Offset
                                           & static_cast<uint64_t>(0xffff'ffff'ffff))) {
                    buffer = std::make_unique<char[]>(fileData->CompressedSize);
                    if (buffer) {
                        if (pka.Handle.Read(buffer.get(), fileData->CompressedSize)
                            != fileData->CompressedSize) {
                            buffer.reset();
                        }
                    }
                }
            }
            if (buffer
                && SenLib::ExtractAndDecompressPkgFile(
                    decompressedData,
                    uncompressedSize,
                    buffer.get(),
                    fileData->CompressedSize,
                    fileData->Flags,
                    HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                return uncompressedSize;
            }
        }
        return 0;
    }

    if (!SenLib::ExtractAndDecompressPkgFile(decompressedData,
                                             uncompressedSize,
                                             compressedData,
                                             compressedSize,
                                             flags,
                                             HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        return 0;
    }

    return uncompressedSize;
}

static void* SetupHacks(HyoutaUtils::Logger& logger,
                        SenLib::TX::GameVersion version,
                        void* codeBase,
                        char* newPage,
                        size_t newPageLength);

namespace {
struct InterceptDecryptData {
    HyoutaUtils::Logger logger;
    void* codeBase;
    char* newPage;
    size_t newPageLength;
};
std::atomic<InterceptDecryptData*> s_InterceptDecryptData = nullptr;
} // namespace

static void SetupHacksAfterDecrypt() {
    InterceptDecryptData* data = s_InterceptDecryptData.load();
    s_InterceptDecryptData.store(nullptr);
    HyoutaUtils::Logger logger = std::move(data->logger);
    void* codeBase = data->codeBase;
    char* newPage = data->newPage;
    size_t newPageLength = data->newPageLength;
    delete data;
    SetupHacks(logger, SenLib::TX::GameVersion::Steam, codeBase, newPage, newPageLength);
}

static void InterceptDecryptThreadFunc(void* codeBase) {
    // very ugly but I'm not sure if there's a better way to do this?
    // spinloop and wait for the entry point to be decrypted
    char* entryPoint = reinterpret_cast<char*>(codeBase) + (0x88e5c4 - 0x401000);
    volatile uint32_t* refpt = reinterpret_cast<uint32_t*>(entryPoint);
    while (!(*refpt == (uint32_t)0xe9 && *(refpt + 1) == (uint32_t)0x68146a00)) {
        ;
    }

    // call SetupHacksAfterDecrypt at entry point
    void* funcptr = SetupHacksAfterDecrypt;
    int32_t diff = (reinterpret_cast<char*>(funcptr) - (entryPoint + 5));
    *entryPoint = static_cast<char>(0xe8);
    std::memcpy(entryPoint + 1, &diff, 4);

    FlushInstructionCache(GetCurrentProcess(), entryPoint, 5);
}

static void* SetupHacksInit(HyoutaUtils::Logger& logger) {
    void* codeBase = nullptr;
    const auto maybeVersion = FindImageBase(logger, &codeBase);
    if (!maybeVersion || !codeBase) {
        logger.Log("Failed finding Tokyo Xanadu executable in memory -- wrong game or version?\n");
        LogMemoryMap(logger);
        return nullptr;
    }

    // allocate extra page for code
    const size_t newPageLength = 0x1000;
    char* newPage =
        static_cast<char*>(VirtualAlloc(nullptr, newPageLength, MEM_COMMIT, PAGE_READWRITE));
    if (!newPage) {
        logger.Log("VirtualAlloc failed, skipping remaining patches.\n");
        return nullptr;
    }
    std::memset(newPage, 0xcc, newPageLength);

    const GameVersion version = *maybeVersion;
    if (version == GameVersion::Steam_Encrypted) {
        logger.Log("Encrypted Steam version, attempting to intercept decryption...\n");

        s_InterceptDecryptData.store(
            new InterceptDecryptData{std::move(logger), codeBase, newPage, newPageLength});
        std::thread(InterceptDecryptThreadFunc, codeBase).detach();

        return nullptr;
    } else {
        return SetupHacks(logger, version, codeBase, newPage, newPageLength);
    }
}

static void* SetupHacks(HyoutaUtils::Logger& logger,
                        SenLib::TX::GameVersion version,
                        void* codeBase,
                        char* newPage,
                        size_t newPageLength) {
    s_TrackedMalloc = reinterpret_cast<PTrackedMalloc>(
        static_cast<char*>(codeBase)
        + (version == GameVersion::Steam ? (0x7c1190 - 0x401000) : (0x7bf8d0 - 0x401000)));
    s_TrackedFree = reinterpret_cast<PTrackedFree>(
        static_cast<char*>(codeBase)
        + (version == GameVersion::Steam ? (0x7c1110 - 0x401000) : (0x7bf850 - 0x401000)));
    s_PrFileBufferMalloc = reinterpret_cast<PPrFileMalloc>(
        static_cast<char*>(codeBase)
        + (version == GameVersion::Steam ? (0x40ac70 - 0x401000) : (0x409d90 - 0x401000)));
    s_Ptr_PrFileBufferFree = reinterpret_cast<PPPrFileFree>(
        static_cast<char*>(codeBase)
        + (version == GameVersion::Steam ? (0x9781ec - 0x401000) : (0x9771ec - 0x401000)));

    char* newPageStart = newPage;
    char* newPageEnd = newPage + newPageLength;

    // TX always runs in root
    std::string_view baseDirUtf8 = ".";

    // reserve 0x200 bytes at the start of newPage for use as a bitfield lookup for which DLC IDs
    // are valid to be loaded
    static constexpr size_t dlcValidBitfieldSize = 0x200;
    static constexpr size_t maxDlcId = dlcValidBitfieldSize * 8;
    char* dlcValidBitfield = newPage;
    newPage += dlcValidBitfieldSize;

    // initialize to a sane default: all IDs that are not used by any of the DLCs included in the
    // game, plus the 6 IDs that are checked by the unpatched game
    std::memset(dlcValidBitfield, 0xff, dlcValidBitfieldSize);
    dlcValidBitfield[21] = static_cast<char>(0xcf);
    dlcValidBitfield[22] = static_cast<char>(0xfc);
    dlcValidBitfield[26] = static_cast<char>(0x03);
    dlcValidBitfield[27] = static_cast<char>(0xfe);

    bool assetFixes = true;
    bool disableMouseCamera = false;
    bool showMouseCursor = false;
    bool enableBackgroundControllerInput = false;
    bool useJapaneseLanguage = false;
    bool skipLogos = true;
    bool skipAllMovies = false;
    bool makeTurboToggle = false;
    float turboModeFactor = 2.0f;
    bool forceRegularMG04UVs = false;
    bool enableWavFiles = false;
    bool preferFFileHandle = false;
    bool fixBgmResume = true;
    bool allowCustomDlcMultiuse = true;
    bool disableCameraAutoCenter = false;

    {
        std::string settingsFilePath;
        settingsFilePath.reserve(baseDirUtf8.size() + 24);
        settingsFilePath.append(baseDirUtf8);
        settingsFilePath.push_back('/');
        settingsFilePath.append("senpatcher_settings.ini");
        HyoutaUtils::IO::File settingsFile(std::string_view(settingsFilePath),
                                           HyoutaUtils::IO::OpenMode::Read);
        if (settingsFile.IsOpen()) {
            HyoutaUtils::Ini::IniFile ini;
            if (ini.ParseFile(settingsFile)) {
                const auto check_boolean =
                    [&](std::string_view section, std::string_view key, bool& b) {
                        const auto* kvp = ini.FindValue(section, key);
                        if (kvp) {
                            using HyoutaUtils::TextUtils::CaseInsensitiveEquals;
                            if (CaseInsensitiveEquals(kvp->Value, "true")) {
                                logger.Log("Patch ");
                                logger.Log(key);
                                logger.Log(" enabled via ini.\n");
                                b = true;
                            } else if (CaseInsensitiveEquals(kvp->Value, "false")) {
                                logger.Log("Patch ");
                                logger.Log(key);
                                logger.Log(" disabled via ini.\n");
                                b = false;
                            } else {
                                logger.Log("Patch ");
                                logger.Log(key);
                                logger.Log(" not found in ini, leaving default.\n");
                            }
                        }
                    };
                const auto check_float =
                    [&](std::string_view section, std::string_view key, float& f) {
                        const auto* kvp = ini.FindValue(section, key);
                        if (kvp) {
                            float floatval = 0.0f;
                            const auto [_, ec] = std::from_chars(
                                kvp->Value.data(), kvp->Value.data() + kvp->Value.size(), floatval);
                            if (ec == std::errc()) {
                                logger.Log("Value ");
                                logger.Log(key);
                                logger.Log(" set to ");
                                logger.LogFloat(floatval);
                                logger.Log(".\n");
                                f = floatval;
                            } else {
                                logger.Log("Value ");
                                logger.Log(key);
                                logger.Log(" not found in ini, leaving default.\n");
                            }
                        }
                    };
                const auto check_language =
                    [&](std::string_view section, std::string_view key, bool& b) {
                        const auto* kvp = ini.FindValue(section, key);
                        if (kvp) {
                            using HyoutaUtils::TextUtils::CaseInsensitiveEquals;
                            if (CaseInsensitiveEquals(kvp->Value, "Japanese")) {
                                logger.Log("Set to Japanese via ini.\n");
                                b = true;
                            } else if (CaseInsensitiveEquals(kvp->Value, "English")) {
                                logger.Log("Set to English via ini.\n");
                            } else {
                                logger.Log("Invalid language selected, defaulting to English.\n");
                            }
                        } else {
                            logger.Log("No language selected, defaulting to English.\n");
                        }
                    };
                check_boolean("TX", "AssetFixes", assetFixes);
                check_boolean("TX", "DisableMouseCamera", disableMouseCamera);
                check_boolean("TX", "ShowMouseCursor", showMouseCursor);
                check_boolean(
                    "TX", "EnableBackgroundControllerInput", enableBackgroundControllerInput);
                check_language("TX", "Language", useJapaneseLanguage);
                check_boolean("TX", "SkipLogos", skipLogos);
                check_boolean("TX", "SkipAllMovies", skipAllMovies);
                check_boolean("TX", "MakeTurboToggle", makeTurboToggle);
                check_float("TX", "TurboModeFactor", turboModeFactor);
                check_boolean("TX", "EnableWavFiles", enableWavFiles);
                check_boolean("TX", "PreferFFileHandle", preferFFileHandle);
                check_boolean("TX", "FixBgmResume", fixBgmResume);
                check_boolean("TX", "AllowCustomDlcMultiuse", allowCustomDlcMultiuse);
                check_boolean("TX", "DisableCameraAutoCenter", disableCameraAutoCenter);

                // read valid DLC bitfields from the ini
                {
                    const auto* kvp = ini.FindValue("TX", "ValidDlcIds");
                    if (kvp) {
                        // reset bitfield
                        std::memset(dlcValidBitfield, 0, dlcValidBitfieldSize);

                        const char* strBegin = kvp->Value.data();
                        const char* strEnd = kvp->Value.data() + kvp->Value.size();
                        while (strBegin != strEnd) {
                            int rangeBegin = 0;
                            const auto [end1, ec1] = std::from_chars(strBegin, strEnd, rangeBegin);
                            if (ec1 != std::errc()) {
                                break;
                            }
                            int rangeEnd = rangeBegin;
                            const char* tmp = end1;
                            if (tmp != strEnd && *tmp == '-') {
                                const auto [end2, ec2] = std::from_chars(tmp + 1, strEnd, rangeEnd);
                                if (ec2 != std::errc()) {
                                    break;
                                }
                                tmp = end2;
                            }

                            rangeBegin = std::clamp(rangeBegin, 0, 4095);
                            rangeEnd = std::clamp(rangeEnd, 0, 4095);
                            if (rangeBegin <= rangeEnd) {
                                logger.Log("Making DLC ");
                                if (rangeBegin != rangeEnd) {
                                    logger.Log("range ");
                                    logger.LogInt(rangeBegin);
                                    logger.Log("-");
                                }
                                logger.LogInt(rangeEnd);
                                logger.Log(" valid.\n");

                                for (int i = rangeBegin; i <= rangeEnd; ++i) {
                                    dlcValidBitfield[i >> 3] |= static_cast<char>(1 << (i & 7));
                                }
                            }

                            if (tmp != strEnd && *tmp == ',') {
                                strBegin = tmp + 1;
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    s_JapaneseLanguage = useJapaneseLanguage;

    SenLib::ModLoad::CreateModDirectory(baseDirUtf8);

    // misc_jp.p3a is only loaded when using Japanese, where it is loaded first and overrides some
    // misc.p3a files for scripts/text
    std::span<const std::string_view> vanillaP3aNames =
        useJapaneseLanguage ? s_VanillaP3ANames
                            : std::span<const std::string_view>(s_VanillaP3ANames.data() + 1,
                                                                s_VanillaP3ANames.size() - 1);
    LoadP3As(logger, s_LoadedVanillaP3As, baseDirUtf8, vanillaP3aNames);
    LoadPkas(logger, s_LoadedPkaData, baseDirUtf8, s_PkaGroupPrefixes, s_PkaNames);

    if (!useJapaneseLanguage && s_LoadedPkaData.Groups) {
        // skip checking the Japanese pkgs if we're running in English
        s_LoadedPkaData.Groups[0].PkgCount = 0;
    }

    bool assetCreationSuccess = true;
    if (assetFixes) {
        assetCreationSuccess = SenLib::TX::CreateAssetPatchIfNeeded(logger,
                                                                    baseDirUtf8,
                                                                    s_LoadedVanillaP3As,
                                                                    s_LoadedPkaData,
                                                                    s_PkaGroupPrefixes,
                                                                    useJapaneseLanguage);
    }

    LoadModP3As(logger,
                s_LoadedModsData,
                baseDirUtf8,
                assetFixes && assetCreationSuccess,
                "TXMod",
                useJapaneseLanguage,
                [&](const HyoutaUtils::Ini::IniFile& ini) {
                    if (!forceRegularMG04UVs) {
                        const auto* kvp = ini.FindValue("TXMod", "ForceRegularMG04UVs");
                        if (kvp) {
                            using HyoutaUtils::TextUtils::CaseInsensitiveEquals;
                            if (CaseInsensitiveEquals(kvp->Value, "true")) {
                                logger.Log("ForceRegularMG04UVs");
                                logger.Log(" enabled via mod.\n");
                                forceRegularMG04UVs = true;
                            }
                        }
                    };
                });

    SenLib::TX::PatchExecData patchExecData;
    patchExecData.Logger = &logger;
    patchExecData.TextRegion = static_cast<char*>(codeBase);
    patchExecData.Version = version;
    patchExecData.Codespace = newPage;
    patchExecData.CodespaceEnd = newPageEnd;

    SenLib::TX::OverrideLanguage(patchExecData, useJapaneseLanguage);

    if (preferFFileHandle) {
        SenLib::TX::InjectAtFFileOpen(patchExecData, &FFileOpenForwarder);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    SenLib::TX::InjectAtFFileGetFilesize(patchExecData, &FFileGetFilesizeForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::TX::InjectAtDecompressPkg(patchExecData, &DecompressPkgForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::TX::InjectAtPrFileOpen(patchExecData, &PrFileOpenForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::TX::InjectAtPrFileGetFilesize(patchExecData, &FFileGetFilesizeForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::TX::InjectAtPrFileExists(patchExecData, &FFileGetFilesizeForwarder);
    Align16CodePage(logger, patchExecData.Codespace);

#ifdef DEBUG_PR_FILE_LIFETIME
    SenLib::TX::InjectDebugCodeForPrFileLifetime(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);
#endif

    SenLib::TX::AddSenPatcherVersionToTitle(patchExecData, s_LoadedModsData, !assetCreationSuccess);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::TX::PatchValidDlcIds(patchExecData, dlcValidBitfield, maxDlcId);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::TX::PatchCustomDlcAllow(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);

    SenLib::TX::PatchSkipMovies(patchExecData, skipLogos, skipAllMovies);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::TX::PatchTurboAndButtonMappings(
        patchExecData, makeTurboToggle, turboModeFactor, useJapaneseLanguage);
    Align16CodePage(logger, patchExecData.Codespace);

    if (disableMouseCamera) {
        SenLib::TX::PatchDisableMouseCamera(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (showMouseCursor) {
        SenLib::TX::PatchShowMouseCursor(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (enableBackgroundControllerInput) {
        SenLib::TX::PatchEnableBackgroundControllerInput(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (forceRegularMG04UVs) {
        SenLib::TX::ForceMG04UVs(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (enableWavFiles) {
        SenLib::TX::PatchEnableWav(patchExecData, &FFileGetFilesizeForwarder);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (fixBgmResume) {
        SenLib::TX::PatchBgmResume(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (allowCustomDlcMultiuse) {
        PatchCustomDlcMultiuse(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (disableCameraAutoCenter) {
        PatchDisableCameraAutoCenter(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }

    // mark newly allocated page as executable
    {
        DWORD tmpdword;
        VirtualProtect(newPageStart, newPageLength, PAGE_EXECUTE_READ, &tmpdword);
        FlushInstructionCache(GetCurrentProcess(), newPageStart, newPageLength);
    }

    logger.Log("Injection done!\n");

    return newPageStart;
}

PDirectInput8Create InjectionDllInitializer() {
    HyoutaUtils::Logger logger(HyoutaUtils::IO::File(std::string_view("senpatcher_inject_tx.log"),
                                                     HyoutaUtils::IO::OpenMode::Write));
    logger.Log("Initializing Tokyo Xanadu hook from SenPatcher, version " SENPATCHER_VERSION
               "...\n");
    auto* forwarder = LoadForwarderAddress(logger);
    SetupHacksInit(logger);
    return forwarder;
}
static PDirectInput8Create addr_PDirectInput8Create = InjectionDllInitializer();

extern "C" {
HRESULT __stdcall DirectInput8Create(HINSTANCE hinst,
                                     DWORD dwVersion,
                                     REFIID riidltf,
                                     LPVOID* ppvOut,
                                     void* punkOuter) {
    PDirectInput8Create addr = addr_PDirectInput8Create;
    if (!addr) {
        return 0x8007000EL; // DIERR_OUTOFMEMORY
    }
    return addr(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

// Old API for DInput <= 7.0
// We don't actually use this, but exporting this symbol makes GOG Galaxy think we're an old version
// of DInput, which avoids the crash for the GOG Galaxy overlay described in issue #142.
HRESULT __stdcall DirectInputCreateEx(HINSTANCE hinst,
                                      DWORD dwVersion,
                                      REFIID riidltf,
                                      LPVOID* ppvOut,
                                      void* punkOuter) {
    return 0x8007000EL; // DIERR_OUTOFMEMORY
}
}
