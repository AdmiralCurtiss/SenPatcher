#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "util/file.h"
#include "util/hash/crc32.h"
#include "util/ini.h"
#include "util/logger.h"

#include "modload/loaded_mods.h"
#include "sen/pka.h"
#include "sen/pkg.h"
#include "sen/pkg_extract.h"
#include "sen1/exe_patch.h"
#include "sen1/file_fixes.h"
#include "sen1/inject_modloader.h"
#include "util/text.h"

#include "senpatcher_version.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using SenLib::Sen1::GameVersion;

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
            if (info.RegionSize == 0x737000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x44014c - 0x401000), 0x24);
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0xb32f56 - 0x401000), 0x10);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x26477c77) {
                    logger.Log("Appears to be the EN version.\n");
                    *code = info.BaseAddress;
                    return GameVersion::English;
                }
            } else if (info.RegionSize == 0x735000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x43fffc - 0x401000), 0x24);
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0xb31326 - 0x401000), 0x10);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x26477c77) {
                    logger.Log("Appears to be the JP version.\n");
                    *code = info.BaseAddress;
                    return GameVersion::Japanese;
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
    void* MemoryPointer;

    // if nonzero it will be free'd when the struct is released
    void* MemoryPointerForFreeing;

    uint32_t MemoryPosition;
};
static_assert(offsetof(FFile, Filesize) == 0x4);
static_assert(offsetof(FFile, NativeFileHandle) == 0x8);
static_assert(offsetof(FFile, MemoryPointer) == 0xc);
static_assert(offsetof(FFile, MemoryPointerForFreeing) == 0x10);
static_assert(offsetof(FFile, MemoryPosition) == 0x14);

using PTrackedMalloc = void*(__cdecl*)(uint32_t size, uint32_t alignment);
using PTrackedFree = void(__cdecl*)(void* memory);

static PTrackedMalloc s_TrackedMalloc = nullptr;
static PTrackedFree s_TrackedFree = nullptr;

static SenLib::ModLoad::LoadedModsData s_LoadedModsData{};

struct LoadedPkaData {
    HyoutaUtils::IO::File PrimaryPka;
    std::unique_ptr<SenLib::PkaPkgToHashData[]> PrimaryPkgs;
    size_t PrimaryPkgCount = 0;
    std::unique_ptr<SenLib::PkaFileHashData[]> PrimaryPkgFiles;
    size_t PrimaryPkgFilesCount = 0;
    HyoutaUtils::IO::File SecondaryPka;
    std::unique_ptr<SenLib::PkaPkgToHashData[]> SecondaryPkgs;
    size_t SecondaryPkgCount = 0;
    std::unique_ptr<SenLib::PkaFileHashData[]> SecondaryPkgFiles;
    size_t SecondaryPkgFilesCount = 0;

    std::unique_ptr<SenLib::PkaHashToFileData[]> Files;
    size_t FilesCount = 0;
};
static LoadedPkaData s_LoadedPkaData{};

#define PRIMARY_PKA_PATH "data/asset/D3D11.pka"
#define PRIMARY_PKG_PREFIX "data/asset/d3d11/"
#define SECONDARY_PKA_PATH "data/asset/D3D11_us.pka"
#define SECONDARY_PKG_PREFIX "data/asset/d3d11_us/"

// ignore any path that doesn't begin with the 'data' directory
static bool IsValidReroutablePath(const char* path) {
    return (path[0] == 'D' || path[0] == 'd') && (path[1] == 'A' || path[1] == 'a')
           && (path[2] == 'T' || path[2] == 't') && (path[3] == 'A' || path[3] == 'a')
           && (path[4] == '/' || path[4] == '\\');
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

    const SenLib::ModLoad::P3AFileRef* refptr = FindP3AFileRef(s_LoadedModsData, filteredPath);
    if (refptr != nullptr) {
        void* memory = nullptr;
        uint64_t filesize = 0;
        if (!SenLib::ModLoad::ExtractP3AFileToMemory(
                *refptr,
                0x8000'0000,
                memory,
                filesize,
                [](size_t length) { return s_TrackedMalloc(length, 8); },
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
            // first check for the real PKG
            HyoutaUtils::IO::File file(std::string_view(path), HyoutaUtils::IO::OpenMode::Read);
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

            // then check for data in the PKA
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
                void* memory = s_TrackedMalloc(length, 8);
                if (!memory) {
                    ffile->NativeFileHandle = INVALID_HANDLE_VALUE;
                    return 0;
                }

                // build fake pkg
                char* header = (char*)memory;
                std::memcpy(header, &pkaPkg->PkgName[pkaPkg->PkgName.size() - 4], 4);
                std::memcpy(header + 4, &pkaPkg->FileCount, 4);
                header += 8;
                char* data = header + (pkaPkg->FileCount * 0x50);
                uint32_t dataPosition = 8 + (pkaPkg->FileCount * 0x50);
                for (size_t i = 0; i < pkaPkg->FileCount; ++i) {
                    const SenLib::PkaFileHashData& fileHashData = pkgFiles[pkaPkg->FileOffset + i];
                    const SenLib::PkaHashToFileData* fileData = SenLib::FindFileInPkaByHash(
                        pkaData.Files.get(), pkaData.FilesCount, fileHashData.Hash);
                    if (!fileData) {
                        s_TrackedFree(memory);
                        ffile->NativeFileHandle = INVALID_HANDLE_VALUE;
                        return 0;
                    }
                    std::array<uint32_t, 4> headerData;
                    headerData[0] = fileData->UncompressedSize;
                    headerData[1] = 0x20; // compressed size, always the SHA256 hash
                    headerData[2] = dataPosition;
                    headerData[3] = 0x80; // fake flags to indicate that it should look in the PKA

                    assert(fileHashData.Filename.size() == 0x40);
                    assert(fileHashData.Hash.size() == 0x20);
                    std::memcpy(header, fileHashData.Filename.data(), fileHashData.Filename.size());
                    std::memcpy(header + 0x40, headerData.data(), 0x10);
                    std::memcpy(data, fileHashData.Hash.data(), fileHashData.Hash.size());

                    header += 0x50;
                    data += 0x20;
                    dataPosition += 0x20;
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

    int32_t pkaCheckResult1 = checkPka(pkaData.PrimaryPkgs.get(),
                                       pkaData.PrimaryPkgCount,
                                       pkaData.PrimaryPkgFiles.get(),
                                       pkaData.PrimaryPkgFilesCount,
                                       PRIMARY_PKG_PREFIX,
                                       sizeof(PRIMARY_PKG_PREFIX) - 1);
    if (pkaCheckResult1 >= 0) {
        return pkaCheckResult1;
    }
    int32_t pkaCheckResult2 = checkPka(pkaData.SecondaryPkgs.get(),
                                       pkaData.SecondaryPkgCount,
                                       pkaData.SecondaryPkgFiles.get(),
                                       pkaData.SecondaryPkgFilesCount,
                                       SECONDARY_PKG_PREFIX,
                                       sizeof(SECONDARY_PKG_PREFIX) - 1);
    if (pkaCheckResult2 >= 0) {
        return pkaCheckResult2;
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

    const SenLib::ModLoad::P3AFileRef* refptr = FindP3AFileRef(s_LoadedModsData, filteredPath);
    if (refptr != nullptr) {
        const SenLib::ModLoad::P3AFileRef& ref = *refptr;
        const SenPatcher::P3AFileInfo& fi = *ref.FileInfo;
        if (out_filesize) {
            *out_filesize = static_cast<uint32_t>(fi.UncompressedSize);
        }
        return 1;
    }

    const auto checkPka = [&](SenLib::PkaPkgToHashData* pkgs,
                              size_t pkgCount,
                              const char* pkgPrefix,
                              size_t pkgPrefixLength) -> int32_t {
        if (pkgCount > 0 && memcmp(pkgPrefix, filteredPath.data(), pkgPrefixLength) == 0) {
            // first check for the real PKG
            HyoutaUtils::IO::File file(std::string_view(path), HyoutaUtils::IO::OpenMode::Read);
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

            // then check for data in the PKA
            const size_t start = pkgPrefixLength;
            assert(filteredPath.size() - start >= pkgs[0].PkgName.size());
            const SenLib::PkaPkgToHashData* pkaPkg =
                SenLib::FindPkgInPkaByName(pkgs, pkgCount, &filteredPath[start]);
            if (pkaPkg) {
                if (out_filesize) {
                    // this pkg isn't actually real, but its size when crafted is going to be:
                    // 8 bytes fixed header
                    // 0x50 bytes header per file
                    // 0x20 bytes data per file (the SHA256 hash)
                    *out_filesize = 8 + (pkaPkg->FileCount * (0x50 + 0x20));
                }
                return 1;
            }
        }
        return -1;
    };

    const auto& pkaData = s_LoadedPkaData;
    int32_t pkaCheckResult1 = checkPka(pkaData.PrimaryPkgs.get(),
                                       pkaData.PrimaryPkgCount,
                                       PRIMARY_PKG_PREFIX,
                                       sizeof(PRIMARY_PKG_PREFIX) - 1);
    if (pkaCheckResult1 >= 0) {
        return pkaCheckResult1;
    }
    int32_t pkaCheckResult2 = checkPka(pkaData.SecondaryPkgs.get(),
                                       pkaData.SecondaryPkgCount,
                                       SECONDARY_PKG_PREFIX,
                                       sizeof(SECONDARY_PKG_PREFIX) - 1);
    if (pkaCheckResult2 >= 0) {
        return pkaCheckResult2;
    }

    return -1;
}

static int32_t __fastcall FFileOpenForwarder(FFile* ffile, const char* path) {
    return OpenModFile(ffile, path);
}

static int32_t __fastcall FFileGetFilesizeForwarder(const char* path, uint32_t* out_filesize) {
    return GetFilesizeOfModFile(path, out_filesize);
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
        const SenLib::PkaHashToFileData* fileData =
            SenLib::FindFileInPkaByHash(pkaData.Files.get(), pkaData.FilesCount, compressedData);
        if (fileData && fileData->UncompressedSize == uncompressedSize) {
            auto& pka = (fileData->Offset & static_cast<uint64_t>(0x8000'0000'0000'0000))
                            ? pkaData.SecondaryPka
                            : pkaData.PrimaryPka;
            if (pka.SetPosition(fileData->Offset & static_cast<uint64_t>(0x7fff'ffff'ffff'ffff))) {
                auto buffer = std::make_unique<char[]>(fileData->CompressedSize);
                if (buffer) {
                    if (pka.Read(buffer.get(), fileData->CompressedSize)
                        == fileData->CompressedSize) {
                        if (SenLib::ExtractAndDecompressPkgFile(
                                decompressedData,
                                uncompressedSize,
                                buffer.get(),
                                fileData->CompressedSize,
                                fileData->Flags,
                                HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                            return uncompressedSize;
                        }
                    }
                }
            }
        }
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

std::optional<HyoutaUtils::IO::File>
    LoadPka(HyoutaUtils::Logger& logger, SenLib::PkaHeader& pka, std::string_view path) {
    HyoutaUtils::IO::File f(path, HyoutaUtils::IO::OpenMode::Read);
    bool success = f.IsOpen() && SenLib::ReadPkaFromFile(pka, f);
    if (success) {
        // make pkg names consistent
        for (size_t i = 0; i < pka.PkgCount; ++i) {
            auto& pkgName = pka.Pkgs[i].PkgName;
            SenLib::ModLoad::FilterP3APath(pkgName.data(), pkgName.size());
        }

        // sort the pkgs for binary lookup
        std::stable_sort(
            pka.Pkgs.get(),
            pka.Pkgs.get() + pka.PkgCount,
            [](const SenLib::PkaPkgToHashData& lhs, const SenLib::PkaPkgToHashData& rhs) {
                return strncmp(lhs.PkgName.data(), rhs.PkgName.data(), lhs.PkgName.size()) < 0;
            });

        // also need to sort the files but we do that later...
        return f;
    }
    return std::nullopt;
}

void LoadPkas(HyoutaUtils::Logger& logger, LoadedPkaData& loadedPkaData, std::string_view baseDir) {
    constexpr static size_t longestPkaPath = sizeof(PRIMARY_PKA_PATH) > sizeof(SECONDARY_PKA_PATH)
                                                 ? sizeof(PRIMARY_PKA_PATH)
                                                 : sizeof(SECONDARY_PKA_PATH);
    std::string pkaFilePath;
    pkaFilePath.reserve(baseDir.size() + longestPkaPath);
    pkaFilePath.append(baseDir);
    pkaFilePath.push_back('/');
    pkaFilePath.append(PRIMARY_PKA_PATH);
    SenLib::PkaHeader primaryPka;
    auto primaryPkaFile = LoadPka(logger, primaryPka, pkaFilePath);
    if (!primaryPkaFile) {
        return;
    }

    pkaFilePath.clear();
    pkaFilePath.append(baseDir);
    pkaFilePath.push_back('/');
    pkaFilePath.append(SECONDARY_PKA_PATH);
    SenLib::PkaHeader secondaryPka;
    auto secondaryPkaFile = LoadPka(logger, secondaryPka, pkaFilePath);
    if (!secondaryPkaFile) {
        return;
    }

    // mark the files from the secondary pka by setting the high bit of the offset
    for (size_t i = 0; i < secondaryPka.FilesCount; ++i) {
        secondaryPka.Files[i].Offset |= static_cast<uint64_t>(0x8000'0000'0000'0000);
    }

    // build a combined files list
    size_t filesCount = primaryPka.FilesCount + secondaryPka.FilesCount;
    auto files = std::make_unique<SenLib::PkaHashToFileData[]>(filesCount);
    auto it = std::copy(
        primaryPka.Files.get(), primaryPka.Files.get() + primaryPka.FilesCount, files.get());
    std::copy(secondaryPka.Files.get(), secondaryPka.Files.get() + secondaryPka.FilesCount, it);

    // sort the file hashes for binary lookup
    std::stable_sort(
        files.get(),
        files.get() + filesCount,
        [](const SenLib::PkaHashToFileData& lhs, const SenLib::PkaHashToFileData& rhs) {
            return memcmp(lhs.Hash.data(), rhs.Hash.data(), lhs.Hash.size()) < 0;
        });

    // store the collected data
    loadedPkaData.PrimaryPka = std::move(*primaryPkaFile);
    loadedPkaData.PrimaryPkgs = std::move(primaryPka.Pkgs);
    loadedPkaData.PrimaryPkgCount = primaryPka.PkgCount;
    loadedPkaData.PrimaryPkgFiles = std::move(primaryPka.PkgFiles);
    loadedPkaData.PrimaryPkgFilesCount = primaryPka.PkgFilesCount;
    loadedPkaData.SecondaryPka = std::move(*secondaryPkaFile);
    loadedPkaData.SecondaryPkgs = std::move(secondaryPka.Pkgs);
    loadedPkaData.SecondaryPkgCount = secondaryPka.PkgCount;
    loadedPkaData.SecondaryPkgFiles = std::move(secondaryPka.PkgFiles);
    loadedPkaData.SecondaryPkgFilesCount = secondaryPka.PkgFilesCount;
    loadedPkaData.Files = std::move(files);
    loadedPkaData.FilesCount = filesCount;
}

static void* SetupHacks(HyoutaUtils::Logger& logger) {
    void* codeBase = nullptr;
    const auto maybeVersion = FindImageBase(logger, &codeBase);
    if (!maybeVersion || !codeBase) {
        logger.Log("Failed finding CS1 executable in memory -- wrong game or version?\n");
        LogMemoryMap(logger);
        return nullptr;
    }

    const GameVersion version = *maybeVersion;
    s_TrackedMalloc = reinterpret_cast<PTrackedMalloc>(
        static_cast<char*>(codeBase)
        + (version == GameVersion::Japanese ? (0x70fb70 - 0x401000) : (0x711440 - 0x401000)));
    s_TrackedFree = reinterpret_cast<PTrackedFree>(
        static_cast<char*>(codeBase)
        + (version == GameVersion::Japanese ? (0x70fb20 - 0x401000) : (0x7113f0 - 0x401000)));

    // allocate extra page for code
    const size_t newPageLength = 0x1000;
    char* newPage =
        static_cast<char*>(VirtualAlloc(nullptr, newPageLength, MEM_COMMIT, PAGE_READWRITE));
    if (!newPage) {
        logger.Log("VirtualAlloc failed, skipping remaining patches.\n");
        return nullptr;
    }
    std::memset(newPage, 0xcc, newPageLength);
    char* newPageStart = newPage;
    char* newPageEnd = newPage + newPageLength;

    // CS1 should always run in the same directory
    std::string_view baseDirUtf8;
    if (HyoutaUtils::IO::FileExists(std::string_view("Sen1Launcher.exe"))) {
        logger.Log("Root game dir is current dir.\n");
        baseDirUtf8 = ".";
    } else {
        logger.Log("Failed finding root game directory.\n");
        return nullptr;
    }

    bool assetFixes = true;
    bool removeTurboSkip = true;
    bool correctLanguageVoiceTables = true;
    bool fixArtsSupportCutin = true;
    bool force0Kerning = false;
    bool disableMouseCapture = false;
    bool showMouseCursor = false;
    bool disablePauseOnFocusLoss = false;
    bool forceXInput = false;
    bool allowR2NotebookShortcut = false;
    int turboModeButton = 7;
    bool fixBgmEnqueue = true;

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
                const auto check_integer =
                    [&](std::string_view section, std::string_view key, int& i) {
                        const auto* kvp = ini.FindValue(section, key);
                        if (kvp) {
                            int intval = 0;
                            const auto [_, ec] = std::from_chars(
                                kvp->Value.data(), kvp->Value.data() + kvp->Value.size(), intval);
                            if (ec == std::errc()) {
                                logger.Log("Value ");
                                logger.Log(key);
                                logger.Log(" set to ");
                                logger.LogInt(intval);
                                logger.Log(".\n");
                                i = intval;
                            } else {
                                logger.Log("Value ");
                                logger.Log(key);
                                logger.Log(" not found in ini, leaving default.\n");
                            }
                        }
                    };
                check_boolean("CS1", "AssetFixes", assetFixes);
                check_boolean("CS1", "RemoveTurboSkip", removeTurboSkip);
                check_boolean("CS1", "CorrectLanguageVoiceTables", correctLanguageVoiceTables);
                check_boolean("CS1", "FixArtsSupportCutin", fixArtsSupportCutin);
                check_boolean("CS1", "Force0Kerning", force0Kerning);
                check_boolean("CS1", "DisableMouseCapture", disableMouseCapture);
                check_boolean("CS1", "ShowMouseCursor", showMouseCursor);
                check_boolean("CS1", "DisablePauseOnFocusLoss", disablePauseOnFocusLoss);
                check_boolean("CS1", "ForceXInput", forceXInput);
                check_boolean("CS1", "AlwaysUseNotebookR2", allowR2NotebookShortcut);
                check_integer("CS1", "TurboModeButton", turboModeButton);
                check_boolean("CS1", "FixBgmEnqueue", fixBgmEnqueue);
            }
        }
    }

    SenLib::ModLoad::CreateModDirectory(baseDirUtf8);

    LoadPkas(logger, s_LoadedPkaData, baseDirUtf8);

    bool assetCreationSuccess = true;
    if (assetFixes) {
        assetCreationSuccess = SenLib::Sen1::CreateAssetPatchIfNeeded(logger, baseDirUtf8);
    }

    LoadModP3As(logger, s_LoadedModsData, baseDirUtf8, assetFixes);

    SenLib::Sen1::PatchExecData patchExecData;
    patchExecData.Logger = &logger;
    patchExecData.TextRegion = static_cast<char*>(codeBase);
    patchExecData.Version = version;
    patchExecData.Codespace = newPage;
    patchExecData.CodespaceEnd = newPageEnd;

    SenLib::Sen1::InjectAtFFileOpen(patchExecData, &FFileOpenForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen1::InjectAtFFileGetFilesize(patchExecData, &FFileGetFilesizeForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen1::InjectAtDecompressPkg(patchExecData, &DecompressPkgForwarder);
    Align16CodePage(logger, patchExecData.Codespace);

    SenLib::Sen1::DeglobalizeMutexes(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen1::AddSenPatcherVersionToTitle(
        patchExecData, s_LoadedModsData, !assetCreationSuccess);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen1::PatchThorMasterQuartzString(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen1::FixTextboxAdvancePrompt(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);

    if (removeTurboSkip) {
        SenLib::Sen1::RemoveTurboAutoSkip(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (allowR2NotebookShortcut) {
        SenLib::Sen1::AllowR2NotebookShortcut(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (turboModeButton >= 0 && turboModeButton <= 0xF) {
        SenLib::Sen1::ChangeTurboModeButton(patchExecData, turboModeButton);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (correctLanguageVoiceTables) {
        SenLib::Sen1::PatchLanguageAppropriateVoiceTables(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (disableMouseCapture) {
        SenLib::Sen1::PatchDisableMouseCapture(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (showMouseCursor) {
        SenLib::Sen1::PatchShowMouseCursor(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (disablePauseOnFocusLoss) {
        SenLib::Sen1::PatchDisablePauseOnFocusLoss(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (fixArtsSupportCutin) {
        SenLib::Sen1::PatchFixArtsSupportCutin(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (force0Kerning) {
        SenLib::Sen1::PatchForce0Kerning(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (forceXInput) {
        SenLib::Sen1::PatchForceXInput(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (fixBgmEnqueue) {
        PatchMusicQueueing(patchExecData);
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
    HyoutaUtils::Logger logger(HyoutaUtils::IO::File(std::string_view("senpatcher_inject_cs1.log"),
                                                     HyoutaUtils::IO::OpenMode::Write));
    logger.Log("Initializing CS1 hook from SenPatcher, version " SENPATCHER_VERSION "...\n");
    auto* forwarder = LoadForwarderAddress(logger);
    SetupHacks(logger);
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
}