#include <array>
#include <bit>
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
#include "sen4/exe_patch.h"
#include "sen4/file_fixes.h"
#include "sen4/inject_modloader.h"
#include "util/text.h"

#include "senpatcher_version.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using SenLib::Sen4::GameVersion;

using PDirectInput8Create =
    HRESULT (*)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, void* punkOuter);
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
    while ((std::bit_cast<uint64_t>(p) & 0xf) != 0) {
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
            if (info.RegionSize == 0x872000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(crc, static_cast<char*>(info.BaseAddress) + 0xae7a0, 0x3d);
                crc = crc_update(crc, static_cast<char*>(info.BaseAddress) + 0x844780, 0x21);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x300c7e9f) {
                    logger.Log("Appears to be the EN version.\n");
                    *code = info.BaseAddress;
                    return GameVersion::English;
                }
            } else if (info.RegionSize == 0x86f000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(crc, static_cast<char*>(info.BaseAddress) + 0xae780, 0x3d);
                crc = crc_update(crc, static_cast<char*>(info.BaseAddress) + 0x842120, 0x21);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x300c7e9f) {
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
    uint32_t Unknown; // padding between members?
    void* NativeFileHandle;
    void* MemoryPointer;

    // if nonzero it will be free'd when the struct is released
    // should be allocated with s_TrackedMalloc -- warning this one has less parameters than CS3's
    void* MemoryPointerForFreeing;

    uint32_t MemoryPosition;
};
static_assert(offsetof(FFile, Filesize) == 0x8);
static_assert(offsetof(FFile, NativeFileHandle) == 0x10);
static_assert(offsetof(FFile, MemoryPointer) == 0x18);
static_assert(offsetof(FFile, MemoryPointerForFreeing) == 0x20);
static_assert(offsetof(FFile, MemoryPosition) == 0x28);

using PTrackedMalloc = void*(__fastcall*)(uint64_t size, uint64_t alignment);
using PTrackedFree = void(__fastcall*)(void* memory);

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

        HyoutaUtils::IO::File file(std::string_view(tmp), HyoutaUtils::IO::OpenMode::Read);
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

        HyoutaUtils::IO::File file(std::string_view(tmp), HyoutaUtils::IO::OpenMode::Read);
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

static int64_t __fastcall FFileOpenForwarder(FFile* ffile, const char* path) {
    if (OpenModFile(ffile, path)) {
        return 1;
    }
    return 0;
}

static int64_t __fastcall FFileGetFilesizeForwarder(const char* path, uint32_t* out_filesize) {
    auto result = GetFilesizeOfModFile(path);
    if (result) {
        if (out_filesize) {
            *out_filesize = static_cast<uint32_t>(*result);
        }
        return 1;
    }
    return 0;
}

struct FSoundFile {
    void* FRead;
    void* FSeek;
    void* FTell;
    void* FClose;
};
static_assert(sizeof(FSoundFile) == 32);

static int32_t __fastcall SenPatcherFile_FSoundFileRead(HyoutaUtils::IO::File* file,
                                                        void* memory,
                                                        int32_t length) {
    if (length < 1) {
        return 0;
    }

    size_t bytesRead = file->Read(memory, static_cast<size_t>(length));
    if (bytesRead == 0) {
        return -128; // indicates end of file, I think?
    }
    return static_cast<int32_t>(bytesRead);
}

static int32_t __fastcall SenPatcherFile_FSoundFileSeek(HyoutaUtils::IO::File* file,
                                                        int64_t position,
                                                        int mode) {
    if (mode >= 0 && mode <= 2) {
        if (file->SetPosition(position, static_cast<HyoutaUtils::IO::SetPositionMode>(mode))) {
            return 0;
        }
    }
    return -1;
}

static int64_t __fastcall SenPatcherFile_FSoundFileTell(HyoutaUtils::IO::File* file) {
    auto position = file->GetPosition();
    if (position) {
        return static_cast<int64_t>(*position);
    }
    return -1;
}

static void __fastcall SenPatcherFile_FSoundFileClose(HyoutaUtils::IO::File* file) {
    std::unique_ptr<HyoutaUtils::IO::File> tmp(file);
    tmp.reset();
}

struct MemoryFile {
    char* Memory;
    size_t Length;
    size_t Position;
};

static int32_t __fastcall MemoryFile_FSoundFileRead(MemoryFile* file,
                                                    void* memory,
                                                    int32_t length) {
    if (length < 1) {
        return 0;
    }

    char* current = file->Memory + file->Position;
    char* end = file->Memory + file->Length;
    size_t toRead = static_cast<size_t>(end - current);
    if (static_cast<size_t>(length) < toRead) {
        toRead = static_cast<size_t>(length);
    }
    if (toRead == 0) {
        return -128; // indicates end of file, I think?
    }

    std::memcpy(memory, current, toRead);
    file->Position += toRead;
    return static_cast<int32_t>(toRead);
}

static int32_t __fastcall MemoryFile_FSoundFileSeek(MemoryFile* file, int64_t position, int mode) {
    int64_t targetPosition;
    if (mode == 0) {
        targetPosition = position;
    } else if (mode == 1) {
        targetPosition = static_cast<int64_t>(file->Position) + position;
    } else if (mode == 2) {
        targetPosition = static_cast<int64_t>(file->Length) + position;
    } else {
        return -1;
    }

    if (targetPosition < 0 || static_cast<uint64_t>(targetPosition) > file->Length) {
        return -1;
    }

    file->Position = static_cast<size_t>(targetPosition);
    return 0;
}

static int64_t __fastcall MemoryFile_FSoundFileTell(MemoryFile* file) {
    return static_cast<int64_t>(file->Position);
}

static void __fastcall MemoryFile_FSoundFileClose(MemoryFile* file) {
    free(file->Memory);
    delete file;
}

static void* __fastcall FSoundOpenForwarder(FSoundFile* soundFile, const char* path) {
    if (!IsValidReroutablePath(path)) {
        return nullptr;
    }

    if (s_LoadedModsData.CheckDevFolderForAssets) {
        auto file = std::make_unique<HyoutaUtils::IO::File>();
        if (file) {
            const size_t path_len = strlen(path);
            std::string tmp;
            tmp.reserve(4 + path_len);
            tmp.append("dev/");
            tmp.append(path);

            file->Open(std::string_view(tmp), HyoutaUtils::IO::OpenMode::Read);
            if (file->IsOpen()) {
                void* handle = file.release();
                soundFile->FRead = &SenPatcherFile_FSoundFileRead;
                soundFile->FSeek = &SenPatcherFile_FSoundFileSeek;
                soundFile->FTell = &SenPatcherFile_FSoundFileTell;
                soundFile->FClose = &SenPatcherFile_FSoundFileClose;
                return handle;
            }
        }
    }

    std::array<char, 0x100> filteredPath;
    if (!SenLib::ModLoad::FilterGamePath(filteredPath.data(), path, filteredPath.size())) {
        return nullptr;
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
                [](size_t length) { return malloc(length); },
                [](void* memory) { free(memory); })) {
            MemoryFile* handle = new MemoryFile();
            handle->Memory = static_cast<char*>(memory);
            handle->Length = filesize;
            handle->Position = 0;
            soundFile->FRead = &MemoryFile_FSoundFileRead;
            soundFile->FSeek = &MemoryFile_FSoundFileSeek;
            soundFile->FTell = &MemoryFile_FSoundFileTell;
            soundFile->FClose = &MemoryFile_FSoundFileClose;
            return handle;
        }
    }

    return nullptr;
}

static void* SetupHacks(HyoutaUtils::Logger& logger) {
    void* codeBase = nullptr;
    const auto maybeVersion = FindImageBase(logger, &codeBase);
    if (!maybeVersion || !codeBase) {
        logger.Log("Failed finding CS4 executable in memory -- wrong game or version?\n");
        LogMemoryMap(logger);
        return nullptr;
    }

    const GameVersion version = *maybeVersion;
    s_TrackedMalloc = reinterpret_cast<PTrackedMalloc>(static_cast<char*>(codeBase)
                                                       + (version == GameVersion::Japanese
                                                              ? (0x14052ed00 - 0x140001000)
                                                              : (0x140531280 - 0x140001000)));
    s_TrackedFree = reinterpret_cast<PTrackedFree>(static_cast<char*>(codeBase)
                                                   + (version == GameVersion::Japanese
                                                          ? (0x14052ece0 - 0x140001000)
                                                          : (0x140531260 - 0x140001000)));

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
    if (HyoutaUtils::IO::FileExists(std::string_view("Sen4Launcher.exe"))) {
        logger.Log("Root game dir is current dir.\n");
        baseDirUtf8 = ".";
    } else if (HyoutaUtils::IO::FileExists(std::string_view("../../Sen4Launcher.exe"))) {
        logger.Log("Root game dir is ../..\n");
        baseDirUtf8 = "../..";
    } else {
        logger.Log("Failed finding root game directory.\n");
        return nullptr;
    }

    bool assetFixes = true;
    bool allowSwitchToNightmare = true;
    bool forceSwapConfirmCancel = false;
    bool forceSwapConfirmCancelToJp = false;
    bool disableMouseCapture = false;
    bool showMouseCursor = false;
    bool disablePauseOnFocusLoss = false;
    bool fixSwappedButtonsWhenDynamicPromptsOff = true;
    bool fixBgmEnqueue = true;
    int increaseDlcCount = 0x1000;
    bool fixDlcCostumeCrash = true;

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
                check_boolean("CS4", "AssetFixes", assetFixes);
                check_boolean("CS4", "AllowSwitchToNightmare", allowSwitchToNightmare);
                check_boolean("CS4", "ForceConfirmCancel", forceSwapConfirmCancel);
                check_boolean("CS4", "ForceConfirmJp", forceSwapConfirmCancelToJp);
                check_boolean("CS4", "DisableMouseCapture", disableMouseCapture);
                check_boolean("CS4", "ShowMouseCursor", showMouseCursor);
                check_boolean("CS4", "DisablePauseOnFocusLoss", disablePauseOnFocusLoss);
                check_boolean("CS4", "FixBgmEnqueue", fixBgmEnqueue);
                check_integer("CS4", "IncreaseDlcCount", increaseDlcCount);
                check_boolean("CS4", "FixDlcCostumeCrash", fixDlcCostumeCrash);
            }
        }
    }

    SenLib::ModLoad::CreateModDirectory(baseDirUtf8);

    bool assetCreationSuccess = true;
    if (assetFixes) {
        assetCreationSuccess = SenLib::Sen4::CreateAssetPatchIfNeeded(logger, baseDirUtf8);
    }

    LoadModP3As(logger, s_LoadedModsData, baseDirUtf8, assetFixes);

    SenLib::Sen4::PatchExecData patchExecData;
    patchExecData.Logger = &logger;
    patchExecData.TextRegion = static_cast<char*>(codeBase);
    patchExecData.Version = version;
    patchExecData.Codespace = newPage;
    patchExecData.CodespaceEnd = newPageEnd;

    SenLib::Sen4::InjectAtFFileOpen(patchExecData, &FFileOpenForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen4::InjectAtFFileGetFilesize(patchExecData, &FFileGetFilesizeForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen4::InjectAtOpenFSoundFile(patchExecData, &FSoundOpenForwarder);
    Align16CodePage(logger, patchExecData.Codespace);

    DeglobalizeMutexes(patchExecData);
    Align16CodePage(logger, patchExecData.Codespace);
    AddSenPatcherVersionToTitle(patchExecData, s_LoadedModsData, !assetCreationSuccess);
    Align16CodePage(logger, patchExecData.Codespace);

    if (allowSwitchToNightmare) {
        AllowSwitchToNightmare(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (disableMouseCapture) {
        PatchDisableMouseCapture(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (showMouseCursor) {
        PatchShowMouseCursor(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (disablePauseOnFocusLoss) {
        PatchDisablePauseOnFocusLoss(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (forceSwapConfirmCancel) {
        PatchForceSwapConfirmCancel(patchExecData, forceSwapConfirmCancelToJp);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (fixSwappedButtonsWhenDynamicPromptsOff) {
        PatchFixPcConfirmCancelWhenSwapped(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (fixBgmEnqueue) {
        PatchMusicQueueing(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (increaseDlcCount >= 0) {
        PatchIncreaseDlcCount(patchExecData, static_cast<uint32_t>(increaseDlcCount));
    }
    if (fixDlcCostumeCrash) {
        PatchDlcCostumeCrash(patchExecData);
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
    HyoutaUtils::Logger logger(HyoutaUtils::IO::File(std::string_view("senpatcher_inject_cs4.log"),
                                                     HyoutaUtils::IO::OpenMode::Write));
    logger.Log("Initializing CS4 hook from SenPatcher, version " SENPATCHER_VERSION "...\n");
    auto* forwarder = LoadForwarderAddress(logger);
    SetupHacks(logger);
    return forwarder;
}
static PDirectInput8Create addr_PDirectInput8Create = InjectionDllInitializer();

extern "C" {
HRESULT DirectInput8Create(HINSTANCE hinst,
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
