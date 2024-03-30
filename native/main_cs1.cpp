#include <array>
#include <bit>
#include <charconv>
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
                    gameVersion = GameVersion::English;
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
        logger.Log("Failed finding CS1 executable in memory -- wrong game or version?\n");
        return nullptr;
    }

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
    if (SenPatcher::IO::FileExists(std::string_view("Sen1Launcher.exe"))) {
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
            }
        }
    }

    SenLib::ModLoad::CreateModDirectory(baseDirUtf8);

    if (assetFixes) {
        SenLib::Sen1::CreateAssetPatchIfNeeded(logger, baseDirUtf8);
    }

    LoadModP3As(logger, s_LoadedModsData, baseDirUtf8);

    SenLib::Sen1::InjectAtFFileOpen(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd, &FFileOpenForwarder);
    Align16CodePage(logger, newPage);
    SenLib::Sen1::InjectAtFFileGetFilesize(logger,
                                           static_cast<char*>(codeBase),
                                           version,
                                           newPage,
                                           newPageEnd,
                                           &FFileGetFilesizeForwarder);
    Align16CodePage(logger, newPage);

    SenLib::Sen1::DeglobalizeMutexes(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    Align16CodePage(logger, newPage);
    SenLib::Sen1::AddSenPatcherVersionToTitle(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd, s_LoadedModsData);
    Align16CodePage(logger, newPage);
    SenLib::Sen1::PatchThorMasterQuartzString(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    Align16CodePage(logger, newPage);
    SenLib::Sen1::FixTextboxAdvancePrompt(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    Align16CodePage(logger, newPage);

    if (removeTurboSkip) {
        SenLib::Sen1::RemoveTurboAutoSkip(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (allowR2NotebookShortcut) {
        SenLib::Sen1::AllowR2NotebookShortcut(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (turboModeButton >= 0 && turboModeButton <= 0xF) {
        SenLib::Sen1::ChangeTurboModeButton(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd, turboModeButton);
        Align16CodePage(logger, newPage);
    }
    if (correctLanguageVoiceTables) {
        SenLib::Sen1::PatchLanguageAppropriateVoiceTables(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (disableMouseCapture) {
        SenLib::Sen1::PatchDisableMouseCapture(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (showMouseCursor) {
        SenLib::Sen1::PatchShowMouseCursor(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (disablePauseOnFocusLoss) {
        SenLib::Sen1::PatchDisablePauseOnFocusLoss(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (fixArtsSupportCutin) {
        SenLib::Sen1::PatchFixArtsSupportCutin(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (force0Kerning) {
        SenLib::Sen1::PatchForce0Kerning(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (forceXInput) {
        SenLib::Sen1::PatchForceXInput(
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
    SenPatcher::Logger logger(SenPatcher::IO::File(std::string_view("senpatcher_inject_cs1.log"),
                                                   SenPatcher::IO::OpenMode::Write));
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
