#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "crc32.h"
#include "file.h"
#include "ini.h"
#include "logger.h"

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
    while ((std::bit_cast<uint64_t>(p) & 0xf) != 0) {
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
            if (info.RegionSize == 0x872000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(crc, static_cast<char*>(info.BaseAddress) + 0xae7a0, 0x3d);
                crc = crc_update(crc, static_cast<char*>(info.BaseAddress) + 0x844780, 0x21);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x300c7e9f) {
                    logger.Log("Appears to be the EN version.\n");
                    *code = info.BaseAddress;
                    gameVersion = GameVersion::English;
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

static int64_t __fastcall FFileOpenForwarder(FFile* ffile,
                                             const char* path,
                                             int unknownThirdParameter) {
    if (OpenModFile(ffile, path)) {
        return 1;
    }
    return 0;
}

static int64_t __fastcall FFileGetFilesizeForwarder(FFile* ffile,
                                                    const char* path,
                                                    int unknownThirdParameter,
                                                    uint32_t* out_filesize) {
    auto result = GetFilesizeOfModFile(path);
    if (result) {
        if (out_filesize) {
            *out_filesize = static_cast<uint32_t>(*result);
        }
        return 1;
    }
    return 0;
}

static int64_t __fastcall FreestandingGetFilesizeForwarder(const char* path,
                                                           int unknownThirdParameter,
                                                           uint32_t* out_filesize) {
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

static int32_t __fastcall SenPatcherFile_FSoundFileRead(SenPatcher::IO::File* file,
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

static int32_t __fastcall SenPatcherFile_FSoundFileSeek(SenPatcher::IO::File* file,
                                                        int64_t position,
                                                        int mode) {
    if (mode >= 0 && mode <= 2) {
        if (file->SetPosition(position, static_cast<SenPatcher::IO::SetPositionMode>(mode))) {
            return 0;
        }
    }
    return -1;
}

static int64_t __fastcall SenPatcherFile_FSoundFileTell(SenPatcher::IO::File* file) {
    auto position = file->GetPosition();
    if (position) {
        return static_cast<int64_t>(*position);
    }
    return -1;
}

static void __fastcall SenPatcherFile_FSoundFileClose(SenPatcher::IO::File* file) {
    std::unique_ptr<SenPatcher::IO::File> tmp(file);
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
        auto file = std::make_unique<SenPatcher::IO::File>();
        if (file) {
            const size_t path_len = strlen(path);
            std::string tmp;
            tmp.reserve(4 + path_len);
            tmp.append("dev/");
            tmp.append(path);

            file->Open(std::string_view(tmp), SenPatcher::IO::OpenMode::Read);
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

static void* SetupHacks(SenPatcher::Logger& logger) {
    void* codeBase = nullptr;
    GameVersion version = FindImageBase(logger, &codeBase);
    if (version == GameVersion::Unknown || !codeBase) {
        logger.Log("Failed finding CS4 executable in memory -- wrong game or version?\n");
        return nullptr;
    }

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
    std::filesystem::path baseDir;
    std::string_view baseDirUtf8;
    if (std::filesystem::exists(L"Sen4Launcher.exe")) {
        logger.Log("Root game dir is current dir.\n");
        baseDir = L"./";
        baseDirUtf8 = ".";
    } else if (std::filesystem::exists(L"../../Sen4Launcher.exe")) {
        logger.Log("Root game dir is ../..\n");
        baseDir = L"../../";
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

    {
        SenPatcher::IO::File settingsFile(baseDir / L"senpatcher_settings.ini",
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
                check_boolean("CS4", "AssetFixes", assetFixes);
                check_boolean("CS4", "AllowSwitchToNightmare", allowSwitchToNightmare);
                check_boolean("CS4", "ForceConfirmCancel", forceSwapConfirmCancel);
                check_boolean("CS4", "ForceConfirmJp", forceSwapConfirmCancelToJp);
                check_boolean("CS4", "DisableMouseCapture", disableMouseCapture);
                check_boolean("CS4", "ShowMouseCursor", showMouseCursor);
                check_boolean("CS4", "DisablePauseOnFocusLoss", disablePauseOnFocusLoss);
            }
        }
    }

    {
        std::error_code ec;
        std::filesystem::create_directory(baseDir / L"mods", baseDir / L"bin", ec);
    }

    if (assetFixes) {
        SenLib::Sen4::CreateAssetPatchIfNeeded(logger, baseDirUtf8);
    }

    LoadModP3As(logger, s_LoadedModsData, baseDirUtf8);

    SenLib::Sen4::InjectAtFFileOpen(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd, &FFileOpenForwarder);
    Align16CodePage(logger, newPage);
    SenLib::Sen4::InjectAtFFileGetFilesize(logger,
                                           static_cast<char*>(codeBase),
                                           version,
                                           newPage,
                                           newPageEnd,
                                           &FFileGetFilesizeForwarder);
    Align16CodePage(logger, newPage);
    SenLib::Sen4::InjectAtFreestandingGetFilesize(logger,
                                                  static_cast<char*>(codeBase),
                                                  version,
                                                  newPage,
                                                  newPageEnd,
                                                  &FreestandingGetFilesizeForwarder);
    Align16CodePage(logger, newPage);
    SenLib::Sen4::InjectAtOpenFSoundFile(
        logger, static_cast<char*>(codeBase), version, newPage, newPageEnd, &FSoundOpenForwarder);
    Align16CodePage(logger, newPage);

    DeglobalizeMutexes(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    Align16CodePage(logger, newPage);
    AddSenPatcherVersionToTitle(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
    Align16CodePage(logger, newPage);

    if (allowSwitchToNightmare) {
        AllowSwitchToNightmare(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (disableMouseCapture) {
        PatchDisableMouseCapture(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (showMouseCursor) {
        PatchShowMouseCursor(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (disablePauseOnFocusLoss) {
        PatchDisablePauseOnFocusLoss(
            logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);
        Align16CodePage(logger, newPage);
    }
    if (forceSwapConfirmCancel) {
        PatchForceSwapConfirmCancel(logger,
                                    static_cast<char*>(codeBase),
                                    version,
                                    newPage,
                                    newPageEnd,
                                    forceSwapConfirmCancelToJp);
        Align16CodePage(logger, newPage);
    }
    if (fixSwappedButtonsWhenDynamicPromptsOff) {
        PatchFixPcConfirmCancelWhenSwapped(
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
    SenPatcher::Logger logger(
        SenPatcher::IO::File(L"senpatcher_inject_cs4.log", SenPatcher::IO::OpenMode::Write));
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
