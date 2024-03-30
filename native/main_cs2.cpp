#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "crc32.h"
#include "file.h"
#include "ini.h"
#include "logger.h"

#include "modload/loaded_mods.h"
#include "sen2/exe_patch.h"
#include "sen2/file_fixes.h"
#include "sen2/inject_modloader.h"
#include "util/text.h"

#include "senpatcher_version.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using SenLib::Sen2::GameVersion;

using PDirectInput8Create = HRESULT(
    __stdcall*)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, void* punkOuter);
static PDirectInput8Create LoadForwarderAddress(SenPatcher::Logger& logger) {
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

static void Align16CodePage(SenPatcher::Logger& logger, char*& new_page) {
    logger.Log("Aligning ").LogPtr(new_page).Log(" to 16 bytes.\n");
    char* p = new_page;
    while ((std::bit_cast<uint32_t>(p) & 0xf) != 0) {
        *p++ = static_cast<char>(0xcc);
    }
    new_page = p;
}

static GameVersion FindImageBase(SenPatcher::Logger& logger, void** code) {
    GameVersion gameVersion = GameVersion::Unknown;
    MEMORY_BASIC_INFORMATION info;
    std::memset(&info, 0, sizeof(info));
    *code = nullptr;
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

        logger.Log("Querying address ")
            .LogHex(address)
            .Log(", got allocation at ")
            .LogPtr(info.AllocationBase)
            .Log(", base ptr ")
            .LogPtr(info.BaseAddress)
            .Log(", size ")
            .LogHex(info.RegionSize)
            .Log(", protection ")
            .LogHex(info.Protect)
            .Log(".\n");
        if (gameVersion == GameVersion::Unknown && info.State == MEM_COMMIT
            && info.Type == MEM_IMAGE) {
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
                    gameVersion = GameVersion::English;
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
                    gameVersion = GameVersion::Japanese;
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

    return gameVersion;
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

// ignore any path that doesn't begin with the 'data' directory
static bool IsValidReroutablePath(const char* path) {
    return (path[0] == 'D' || path[0] == 'd') && (path[1] == 'A' || path[1] == 'a')
           && (path[2] == 'T' || path[2] == 't') && (path[3] == 'A' || path[3] == 'a')
           && (path[4] == '/' || path[4] == '\\');
}

static bool OpenModFile(FFile* ffile, const char* path) {
    if (!IsValidReroutablePath(path)) {
        return false;
    }

    if (s_LoadedModsData.CheckDevFolderForAssets) {
        const size_t path_len = strlen(path);
        std::string tmp;
        tmp.reserve(4 + path_len);
        tmp.append("dev/");
        tmp.append(path);

        SenPatcher::IO::File file(std::string_view(tmp), SenPatcher::IO::OpenMode::Read);
        if (file.IsOpen()) {
            auto length = file.GetLength();
            if (length && *length < 0x8000'0000) {
                ffile->NativeFileHandle = file.ReleaseHandle();
                ffile->Filesize = static_cast<uint32_t>(*length);
                return true;
            }
        }
    }

    std::array<char, 0x100> filteredPath;
    if (!SenLib::ModLoad::FilterGamePath(filteredPath.data(), path, filteredPath.size())) {
        return false;
    }

    const SenLib::ModLoad::P3AFileRef* refptr = FindP3AFileRef(s_LoadedModsData, filteredPath);
    if (refptr != nullptr) {
        void* memory = nullptr;
        uint64_t filesize = 0;
        if (SenLib::ModLoad::ExtractP3AFileToMemory(
                *refptr,
                0x8000'0000,
                memory,
                filesize,
                [](size_t length) { return s_TrackedMalloc(length, 8); },
                [](void* memory) { s_TrackedFree(memory); })) {
            ffile->Filesize = static_cast<uint32_t>(filesize);
            ffile->MemoryPointer = memory;
            ffile->MemoryPointerForFreeing = memory;
            ffile->MemoryPosition = 0;
            return true;
        }
    }

    return false;
}

static std::optional<uint64_t> GetFilesizeOfModFile(const char* path) {
    if (!IsValidReroutablePath(path)) {
        return std::nullopt;
    }

    if (s_LoadedModsData.CheckDevFolderForAssets) {
        const size_t path_len = strlen(path);
        std::string tmp;
        tmp.reserve(4 + path_len);
        tmp.append("dev/");
        tmp.append(path);

        SenPatcher::IO::File file(std::string_view(tmp), SenPatcher::IO::OpenMode::Read);
        if (file.IsOpen()) {
            auto length = file.GetLength();
            if (length && *length < 0x8000'0000) {
                return length;
            }
        }
    }

    std::array<char, 0x100> filteredPath;
    if (!SenLib::ModLoad::FilterGamePath(filteredPath.data(), path, filteredPath.size())) {
        return std::nullopt;
    }

    const SenLib::ModLoad::P3AFileRef* refptr = FindP3AFileRef(s_LoadedModsData, filteredPath);
    if (refptr != nullptr) {
        const SenLib::ModLoad::P3AFileRef& ref = *refptr;
        const SenPatcher::P3AFileInfo& fi = *ref.FileInfo;
        if (fi.UncompressedSize < 0x8000'0000) {
            return fi.UncompressedSize;
        }
    }

    return std::nullopt;
}

static int32_t __fastcall FFileOpenForwarder(FFile* ffile, const char* path) {
    if (OpenModFile(ffile, path)) {
        return 1;
    }
    return 0;
}

static int32_t __fastcall FFileGetFilesizeForwarder(const char* path, uint32_t* out_filesize) {
    auto result = GetFilesizeOfModFile(path);
    if (result) {
        if (out_filesize) {
            *out_filesize = static_cast<uint32_t>(*result);
        }
        return 1;
    }
    return 0;
}

static void* SetupHacks(SenPatcher::Logger& logger) {
    void* codeBase = nullptr;
    GameVersion version = FindImageBase(logger, &codeBase);
    if (version == GameVersion::Unknown || !codeBase) {
        logger.Log("Failed finding CS2 executable in memory -- wrong game or version?\n");
        return nullptr;
    }

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
    if (SenPatcher::IO::FileExists(std::string_view("Sen2Launcher.exe"))) {
        logger.Log("Root game dir is current dir.\n");
        baseDirUtf8 = ".";
    } else if (SenPatcher::IO::FileExists(std::string_view("../../Sen2Launcher.exe"))) {
        logger.Log("Root game dir is ../..\n");
        baseDirUtf8 = "../..";
    } else {
        logger.Log("Failed finding root game directory.\n");
        return nullptr;
    }

    bool assetFixes = true;
    bool removeTurboSkip = true;
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
        SenPatcher::IO::File settingsFile(std::string_view(settingsFilePath),
                                          SenPatcher::IO::OpenMode::Read);
        if (settingsFile.IsOpen()) {
            SenPatcher::IniFile ini;
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

    bool assetCreationSuccess = true;
    if (assetFixes) {
        assetCreationSuccess = SenLib::Sen2::CreateAssetPatchIfNeeded(logger, baseDirUtf8);
    }

    LoadModP3As(logger, s_LoadedModsData, baseDirUtf8);

    SenLib::Sen2::InjectAtFFileOpen(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd, &FFileOpenForwarder);
    Align16CodePage(logger, newPage);
    SenLib::Sen2::InjectAtFFileGetFilesize(logger,
                                           static_cast<char*>(codeBase),
                                           version,
                                           newPage,
                                           newPageEnd,
                                           &FFileGetFilesizeForwarder);
    Align16CodePage(logger, newPage);

    SenLib::Sen2::DeglobalizeMutexes(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    Align16CodePage(logger, newPage);
    SenLib::Sen2::FixGogGalaxy(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    Align16CodePage(logger, newPage);
    SenLib::Sen2::AddSenPatcherVersionToTitle(logger,
                                              static_cast<char*>(codeBase),
                                              version,
                                              newPage,
                                              newPageEnd,
                                              s_LoadedModsData,
                                              !assetCreationSuccess);
    Align16CodePage(logger, newPage);
    SenLib::Sen2::AddCS2ToTitleBar(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    Align16CodePage(logger, newPage);
    SenLib::Sen2::PatchRemoveDebugLeftovers(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    Align16CodePage(logger, newPage);

    if (removeTurboSkip) {
        SenLib::Sen2::RemoveTurboAutoSkip(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (replaceAudioTimingThread) {
        SenLib::Sen2::PatchMusicFadeTiming(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd, 1000);
        Align16CodePage(logger, newPage);
    }
    if (fixBgmEnqueue) {
        SenLib::Sen2::PatchMusicQueueingOnSoundThreadSide(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (correctLanguageVoiceTables) {
        SenLib::Sen2::PatchLanguageAppropriateVoiceTables(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (disableMouseCapture) {
        SenLib::Sen2::PatchDisableMouseCapture(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (showMouseCursor) {
        SenLib::Sen2::PatchShowMouseCursor(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (disablePauseOnFocusLoss) {
        SenLib::Sen2::PatchDisablePauseOnFocusLoss(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (fixControllerMapping) {
        SenLib::Sen2::PatchFixControllerMappings(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (fixArtsSupportCutin) {
        SenLib::Sen2::PatchFixArtsSupportCutin(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (force0Kerning) {
        SenLib::Sen2::PatchForce0Kerning(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (fixBattleScopeCrash) {
        SenLib::Sen2::PatchAddNullCheckBattleScopeCrashMaybe(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (forceXInput) {
        SenLib::Sen2::PatchForceXInput(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
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
    SenPatcher::Logger logger(SenPatcher::IO::File(std::string_view("senpatcher_inject_cs2.log"),
                                                   SenPatcher::IO::OpenMode::Write));
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
}
