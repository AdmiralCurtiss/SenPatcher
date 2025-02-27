#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

#include "util/file.h"
#include "util/hash/crc32.h"
#include "util/ini.h"
#include "util/logger.h"

#include "modload/loaded_mods.h"
#include "modload/loaded_pka.h"
#include "sen/pka.h"
#include "sen/pkg.h"
#include "sen/pkg_extract.h"
#include "sen2/exe_patch.h"
#include "sen2/file_fixes.h"
#include "sen2/inject_modloader.h"
#include "util/text.h"

#include "senpatcher_version.h"

extern "C" {
__declspec(dllexport) char SenPatcherHook[] = "Sen2";
__declspec(dllexport) char SenPatcherVersion[] = SENPATCHER_VERSION;
}

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

using SenLib::Sen2::GameVersion;

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
            if (info.RegionSize == 0x4e9000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x4112bc - 0x401000), 0x31);
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x881620 - 0x401000), 0x25);
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x881649 - 0x401000), 0x4d);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x77d5af45) {
                    logger.Log("Appears to be the EN version.\n");
                    *code = info.BaseAddress;
                    return GameVersion::English;
                }
            } else if (info.RegionSize == 0x4e8000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x41124c - 0x401000), 0x31);
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x8805c0 - 0x401000), 0x25);
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x8805e9 - 0x401000), 0x4d);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x77d5af45) {
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

static SenLib::ModLoad::LoadedP3AData s_LoadedVanillaP3As{};
static constexpr std::array<std::string_view, 5> s_VanillaP3ANames{
    {"misc.p3a", "bgm.p3a", "se.p3a", "voice.p3a", "voice_jp.p3a"}};

static SenLib::ModLoad::LoadedModsData s_LoadedModsData{};

static SenLib::ModLoad::LoadedPkaData s_LoadedPkaData{};
static constexpr std::array<std::string_view, 2> s_PkaGroupPrefixes{
    {"data/asset/d3d11/", "data/asset/d3d11_us/"}};

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
                                                                    true);
        if (pkaCheckResult >= 0) {
            return pkaCheckResult;
        }
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

static void* SetupHacks(HyoutaUtils::Logger& logger) {
    void* codeBase = nullptr;
    const auto maybeVersion = FindImageBase(logger, &codeBase);
    if (!maybeVersion || !codeBase) {
        logger.Log("Failed finding CS2 executable in memory -- wrong game or version?\n");
        LogMemoryMap(logger);
        return nullptr;
    }

    const GameVersion version = *maybeVersion;
    s_TrackedMalloc = reinterpret_cast<PTrackedMalloc>(
        static_cast<char*>(codeBase)
        + (version == GameVersion::Japanese ? (0x6b0450 - 0x401000) : (0x6b14a0 - 0x401000)));
    s_TrackedFree = reinterpret_cast<PTrackedFree>(
        static_cast<char*>(codeBase)
        + (version == GameVersion::Japanese ? (0x6b03e0 - 0x401000) : (0x6b1430 - 0x401000)));

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

    // figure out whether we're running in the root game directory or in the bin/x64 subdirectory
    // and get a relative path to the root game directory
    std::string_view baseDirUtf8;
    if (HyoutaUtils::IO::FileExists(std::string_view("Sen2Launcher.exe"))) {
        logger.Log("Root game dir is current dir.\n");
        baseDirUtf8 = ".";
    } else if (HyoutaUtils::IO::FileExists(std::string_view("../../Sen2Launcher.exe"))) {
        logger.Log("Root game dir is ../..\n");
        baseDirUtf8 = "../..";
    } else {
        logger.Log("Failed finding root game directory.\n");
        return nullptr;
    }

    bool assetFixes = true;
    bool removeTurboSkip = true;
    bool makeTurboToggle = false;
    bool adjustTimersForTurbo = true;
    bool correctLanguageVoiceTables = true;
    bool fixBgmEnqueue = true;
    bool replaceAudioTimingThread = true;
    bool fixControllerMapping = true;
    bool fixArtsSupportCutin = true;
    bool force0Kerning = false;
    bool disableMouseCapture = false;
    bool showMouseCursor = false;
    bool disablePauseOnFocusLoss = false;
    bool forceXInput = false;
    bool fixBattleScopeCrash = true;

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
                check_boolean("CS2", "AssetFixes", assetFixes);
                check_boolean("CS2", "RemoveTurboSkip", removeTurboSkip);
                check_boolean("CS2", "MakeTurboToggle", makeTurboToggle);
                check_boolean("CS2", "AdjustTimersForTurbo", adjustTimersForTurbo);
                check_boolean("CS2", "CorrectLanguageVoiceTables", correctLanguageVoiceTables);
                check_boolean("CS2", "FixBgmEnqueue", fixBgmEnqueue);
                check_boolean("CS2", "ReplaceAudioTimingThread", replaceAudioTimingThread);
                check_boolean("CS2", "FixControllerMapping", fixControllerMapping);
                check_boolean("CS2", "FixArtsSupportCutin", fixArtsSupportCutin);
                check_boolean("CS2", "Force0Kerning", force0Kerning);
                check_boolean("CS2", "DisableMouseCapture", disableMouseCapture);
                check_boolean("CS2", "ShowMouseCursor", showMouseCursor);
                check_boolean("CS2", "DisablePauseOnFocusLoss", disablePauseOnFocusLoss);
                check_boolean("CS2", "ForceXInput", forceXInput);
                check_boolean("CS2", "FixBattleScopeCrash", fixBattleScopeCrash);
            }
        }
    }

    SenLib::ModLoad::CreateModDirectory(baseDirUtf8);

    LoadP3As(logger, s_LoadedVanillaP3As, baseDirUtf8, s_VanillaP3ANames);
    LoadPkas(logger, s_LoadedPkaData, baseDirUtf8, s_PkaGroupPrefixes, {});

    bool assetCreationSuccess = true;
    if (assetFixes) {
        assetCreationSuccess =
            SenLib::Sen2::CreateAssetPatchIfNeeded(logger, baseDirUtf8, s_LoadedVanillaP3As);
    }

    LoadModP3As(logger,
                s_LoadedModsData,
                baseDirUtf8,
                assetFixes,
                "CS2Mod",
                version == GameVersion::Japanese,
                [](const HyoutaUtils::Ini::IniFile& ini) { return; });

    SenLib::Sen2::PatchExecData patchExecData;
    patchExecData.Logger = &logger;
    patchExecData.TextRegion = static_cast<char*>(codeBase);
    patchExecData.Version = version;
    patchExecData.Codespace = newPage;
    patchExecData.CodespaceEnd = newPageEnd;

    SenLib::Sen2::InjectAtFFileOpen(patchExecData, &FFileOpenForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen2::InjectAtFFileGetFilesize(patchExecData, &FFileGetFilesizeForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen2::InjectAtDecompressPkg(patchExecData, &DecompressPkgForwarder);
    Align16CodePage(logger, patchExecData.Codespace);

    SenLib::Sen2::DeglobalizeMutexes(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen2::FixGogGalaxy(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen2::AddSenPatcherVersionToTitle(
        patchExecData, s_LoadedModsData, !assetCreationSuccess);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen2::AddCS2ToTitleBar(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen2::PatchRemoveDebugLeftovers(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen2::DisableCrashReporter(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen2::PatchTurboMode(
        patchExecData, removeTurboSkip, makeTurboToggle, adjustTimersForTurbo);
    Align16CodePage(logger, patchExecData.Codespace);

    if (replaceAudioTimingThread) {
        SenLib::Sen2::PatchMusicFadeTiming(patchExecData, 1000);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (fixBgmEnqueue) {
        SenLib::Sen2::PatchMusicQueueingOnSoundThreadSide(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (correctLanguageVoiceTables) {
        SenLib::Sen2::PatchLanguageAppropriateVoiceTables(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (disableMouseCapture) {
        SenLib::Sen2::PatchDisableMouseCapture(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (showMouseCursor) {
        SenLib::Sen2::PatchShowMouseCursor(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (disablePauseOnFocusLoss) {
        SenLib::Sen2::PatchDisablePauseOnFocusLoss(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (fixControllerMapping) {
        SenLib::Sen2::PatchFixControllerMappings(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (fixArtsSupportCutin) {
        SenLib::Sen2::PatchFixArtsSupportCutin(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (force0Kerning) {
        SenLib::Sen2::PatchForce0Kerning(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (fixBattleScopeCrash) {
        SenLib::Sen2::PatchAddNullCheckBattleScopeCrashMaybe(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (forceXInput) {
        SenLib::Sen2::PatchForceXInput(patchExecData);
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
    HyoutaUtils::Logger logger(HyoutaUtils::IO::File(std::string_view("senpatcher_inject_cs2.log"),
                                                     HyoutaUtils::IO::OpenMode::Write));
    logger.Log("Initializing CS2 hook from SenPatcher, version " SENPATCHER_VERSION "...\n");
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
