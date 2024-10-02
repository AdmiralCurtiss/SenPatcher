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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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

static PTrackedMalloc s_TrackedMalloc = nullptr;
static PTrackedFree s_TrackedFree = nullptr;

static SenLib::ModLoad::LoadedP3AData s_LoadedVanillaP3As{};
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

    std::recursive_mutex PrimaryPkaMutex;
    std::recursive_mutex SecondaryPkaMutex;

    LoadedPkaData() = default;
    LoadedPkaData(const LoadedPkaData& other) = delete;
    LoadedPkaData(LoadedPkaData&& other) = delete;
    LoadedPkaData& operator=(const LoadedPkaData& other) = delete;
    LoadedPkaData& operator=(LoadedPkaData&& other) = delete;
    ~LoadedPkaData() = default;
};
static LoadedPkaData s_LoadedPkaData{};

#define PRIMARY_PKA_PATH "Asset.pka"
#define SECONDARY_PKA_PATH "Japanese.pka"

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
                              size_t pkgFilesCount) -> int32_t {
        if (pkgCount > 0) {
            // check for data in the PKA
            const SenLib::PkaPkgToHashData* pkaPkg =
                SenLib::FindPkgInPkaByName(pkgs, pkgCount, filteredPath.data());
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
                if (!SenLib::ModLoad::BuildFakePkaPkg(
                        (char*)memory, pkaPkg, pkgFiles, pkaData.Files.get(), pkaData.FilesCount)) {
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

    if (s_JapaneseLanguage) {
        int32_t pkaCheckResult2 = checkPka(pkaData.SecondaryPkgs.get(),
                                           pkaData.SecondaryPkgCount,
                                           pkaData.SecondaryPkgFiles.get(),
                                           pkaData.SecondaryPkgFilesCount);
        if (pkaCheckResult2 >= 0) {
            return pkaCheckResult2;
        }
    }
    int32_t pkaCheckResult1 = checkPka(pkaData.PrimaryPkgs.get(),
                                       pkaData.PrimaryPkgCount,
                                       pkaData.PrimaryPkgFiles.get(),
                                       pkaData.PrimaryPkgFilesCount);
    if (pkaCheckResult1 >= 0) {
        return pkaCheckResult1;
    }

    return -1;
}

static int32_t GetPkaPkgFilesizeTX(SenLib::PkaPkgToHashData* pkgs,
                                   size_t pkgCount,
                                   const std::array<char, 0x100>& filteredPath,
                                   const char* path,
                                   uint32_t* out_filesize) {
    if (pkgCount > 0) {
        // check for data in the PKA
        const SenLib::PkaPkgToHashData* pkaPkg =
            SenLib::FindPkgInPkaByName(pkgs, pkgCount, filteredPath.data());
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
    if (s_JapaneseLanguage) {
        int32_t pkaCheckResult2 = GetPkaPkgFilesizeTX(pkaData.SecondaryPkgs.get(),
                                                      pkaData.SecondaryPkgCount,
                                                      filteredPath,
                                                      path,
                                                      out_filesize);
        if (pkaCheckResult2 >= 0) {
            return pkaCheckResult2;
        }
    }
    int32_t pkaCheckResult1 = GetPkaPkgFilesizeTX(
        pkaData.PrimaryPkgs.get(), pkaData.PrimaryPkgCount, filteredPath, path, out_filesize);
    if (pkaCheckResult1 >= 0) {
        return pkaCheckResult1;
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
            std::unique_ptr<char[]> buffer = nullptr;
            {
                bool isSecondary = (fileData->Offset & static_cast<uint64_t>(0x8000'0000'0000'0000))
                                       ? true
                                       : false;
                auto& pka = isSecondary ? pkaData.SecondaryPka : pkaData.PrimaryPka;
                std::lock_guard<std::recursive_mutex> lock(isSecondary ? pkaData.SecondaryPkaMutex
                                                                       : pkaData.PrimaryPkaMutex);
                if (pka.SetPosition(fileData->Offset
                                    & static_cast<uint64_t>(0x7fff'ffff'ffff'ffff))) {
                    buffer = std::make_unique<char[]>(fileData->CompressedSize);
                    if (buffer) {
                        if (pka.Read(buffer.get(), fileData->CompressedSize)
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

static std::optional<HyoutaUtils::IO::File>
    LoadPka(HyoutaUtils::Logger& logger, SenLib::PkaHeader& pka, std::string_view path) {
    logger.Log("Trying to open PKA at ").Log(path).Log("\n");
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
        logger.Log("Opened PKA.\n");
        return f;
    }
    return std::nullopt;
}

static void
    LoadPkas(HyoutaUtils::Logger& logger, LoadedPkaData& loadedPkaData, std::string_view baseDir) {
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

    // Japanese.p3a is only loaded when using Japanese, where it is loaded first and overrides the
    // System.p3a files for scripts/text
    if (useJapaneseLanguage) {
        LoadP3As(logger,
                 s_LoadedVanillaP3As,
                 baseDirUtf8,
                 {{"Japanese.p3a", "System.p3a", "BGM.p3a", "SE.p3a", "Voice.p3a"}});
    } else {
        LoadP3As(logger,
                 s_LoadedVanillaP3As,
                 baseDirUtf8,
                 {{"System.p3a", "BGM.p3a", "SE.p3a", "Voice.p3a"}});
    }
    LoadPkas(logger, s_LoadedPkaData, baseDirUtf8);

    bool assetCreationSuccess = true;
    if (assetFixes) {
        assetCreationSuccess =
            SenLib::TX::CreateAssetPatchIfNeeded(logger, baseDirUtf8, s_LoadedVanillaP3As);
    }

    LoadModP3As(logger,
                s_LoadedModsData,
                baseDirUtf8,
                assetFixes,
                "TXMod",
                useJapaneseLanguage,
                [](const HyoutaUtils::Ini::IniFile& ini) { return; });

    SenLib::TX::PatchExecData patchExecData;
    patchExecData.Logger = &logger;
    patchExecData.TextRegion = static_cast<char*>(codeBase);
    patchExecData.Version = version;
    patchExecData.Codespace = newPage;
    patchExecData.CodespaceEnd = newPageEnd;

    SenLib::TX::OverrideLanguage(patchExecData, useJapaneseLanguage);

    SenLib::TX::InjectAtFFileOpen(patchExecData, &FFileOpenForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::TX::InjectAtFFileGetFilesize(patchExecData, &FFileGetFilesizeForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::TX::InjectAtDecompressPkg(patchExecData, &DecompressPkgForwarder);
    Align16CodePage(logger, patchExecData.Codespace);

    SenLib::TX::AddSenPatcherVersionToTitle(patchExecData, s_LoadedModsData, !assetCreationSuccess);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::TX::PatchValidDlcIds(patchExecData, dlcValidBitfield, maxDlcId);
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
