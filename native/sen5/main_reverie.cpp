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
#include "sen5/exe_patch.h"
#include "sen5/file_fixes.h"
#include "sen5/inject_modloader.h"
#include "util/text.h"

#include "senpatcher_version.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using SenLib::Sen5::GameVersion;

using PDirectSoundCreate8 = HRESULT (*)(void* lpcGuidDevice, void* ppDS8, void* pUnkOuter);
static PDirectSoundCreate8 LoadForwarderAddress(HyoutaUtils::Logger& logger) {
    constexpr int total = 5000;
    WCHAR tmp[total];
    UINT count = GetSystemDirectoryW(tmp, sizeof(tmp) / sizeof(WCHAR));
    if (count == 0 || count > total - 16) {
        logger.Log("Failed constructing path for system dsound.dll.\n");
        return nullptr;
    }
    std::memcpy(tmp + count, L"\\dsound.dll\0", sizeof(L"\\dsound.dll\0"));

    HMODULE dll = ::LoadLibraryW(tmp);
    if (!dll) {
        logger.Log("Failed loading system dsound.dll.\n");
        return nullptr;
    }
    void* addr = ::GetProcAddress(dll, "DirectSoundCreate8");
    if (!addr) {
        logger.Log("Failed finding system DirectSoundCreate8.\n");
    } else {
        logger.Log("Found system DirectSoundCreate8 at ").LogPtr(addr).Log(".\n");
    }
    return (PDirectSoundCreate8)addr;
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
            if (info.RegionSize == 0xbe4000 && info.Protect == PAGE_EXECUTE_READ) {
                crc_t crc = crc_init();
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x140088dd8 - 0x140001000), 0x5a);
                crc = crc_update(
                    crc, static_cast<char*>(info.BaseAddress) + (0x140b73d30 - 0x140001000), 0x92);
                crc = crc_finalize(crc);
                logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                if (crc == 0x054e1c1d) {
                    logger.Log("Appears to be the EN version.\n");
                    *code = info.BaseAddress;
                    return GameVersion::English;
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

// returns:
// 0 if we should report that the file open failed
// 1 if we should report that the file open succeeded
// -1 if the original function should continue running
static int64_t OpenModFile(FFile* ffile, const char* path) {
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

    return -1;
}

// returns:
// 0 if we should report that the file does not exist
// 1 if we should report that the file does exist (in only this case *out_filesize will be set)
// -1 if the original function should continue running
static int64_t GetFilesizeOfModFile(const char* path, uint32_t* out_filesize) {
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
    if (refptr != nullptr) {
        const SenLib::ModLoad::P3AFileRef& ref = *refptr;
        const SenPatcher::P3AFileInfo& fi = *ref.FileInfo;
        if (out_filesize) {
            *out_filesize = static_cast<uint32_t>(fi.UncompressedSize);
        }
        return 1;
    }

    return -1;
}

static int64_t __fastcall FFileOpenForwarder(FFile* ffile, const char* path) {
    return OpenModFile(ffile, path);
}

static int64_t __fastcall FFileExistsForwarder(const char* path) {
    return GetFilesizeOfModFile(path, nullptr);
}

static int64_t __fastcall FFileGetFilesizeForwarder(const char* path, uint32_t* out_filesize) {
    return GetFilesizeOfModFile(path, out_filesize);
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

// returns:
// 0 if we should report that the file open failed
// 1 if we should report that the file open succeeded (*out_handle will contain the file pointer)
// -1 if the original function should continue running
static int64_t __fastcall FSoundOpenForwarder(FSoundFile* soundFile,
                                              const char* path,
                                              void** out_handle) {
    if (!IsValidReroutablePath(path)) {
        return -1;
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
                *out_handle = handle;
                return 1;
            }
        }
    }

    std::array<char, 0x100> filteredPath;
    if (!SenLib::ModLoad::FilterGamePath(filteredPath.data(), path, filteredPath.size())) {
        return -1;
    }

    const SenLib::ModLoad::P3AFileRef* refptr =
        FindP3AFileRef(s_LoadedModsData.LoadedP3As, filteredPath);
    if (refptr != nullptr) {
        void* memory = nullptr;
        uint64_t filesize = 0;
        if (!SenLib::ModLoad::ExtractP3AFileToMemory(
                *refptr,
                0x8000'0000,
                memory,
                filesize,
                [](size_t length) { return malloc(length); },
                [](void* memory) { free(memory); })) {
            return 0;
        }
        MemoryFile* handle = new MemoryFile();
        handle->Memory = static_cast<char*>(memory);
        handle->Length = filesize;
        handle->Position = 0;
        soundFile->FRead = &MemoryFile_FSoundFileRead;
        soundFile->FSeek = &MemoryFile_FSoundFileSeek;
        soundFile->FTell = &MemoryFile_FSoundFileTell;
        soundFile->FClose = &MemoryFile_FSoundFileClose;
        *out_handle = handle;
        return 1;
    }

    return -1;
}

static void* SetupHacks(HyoutaUtils::Logger& logger) {
    void* codeBase = nullptr;
    const auto maybeVersion = FindImageBase(logger, &codeBase);
    if (!maybeVersion || !codeBase) {
        logger.Log("Failed finding Reverie executable in memory -- wrong game or version?\n");
        LogMemoryMap(logger);
        return nullptr;
    }

    const GameVersion version = *maybeVersion;
    s_TrackedMalloc = reinterpret_cast<PTrackedMalloc>(
        SenLib::Sen5::GetCodeAddressEn(version, static_cast<char*>(codeBase), 0x14071fa70));
    s_TrackedFree = reinterpret_cast<PTrackedFree>(
        SenLib::Sen5::GetCodeAddressEn(version, static_cast<char*>(codeBase), 0x14006b660));

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

    std::string_view baseDirUtf8;
    if (HyoutaUtils::IO::DirectoryExists(std::string_view("data"))) {
        logger.Log("Root game dir is current dir.\n");
        baseDirUtf8 = ".";
    } else if (HyoutaUtils::IO::DirectoryExists(std::string_view("../../data"))) {
        logger.Log("Root game dir is ../..\n");
        baseDirUtf8 = "../..";
    } else {
        logger.Log("Failed finding root game directory.\n");
        return nullptr;
    }

    bool assetFixes = true;
    bool fixBgmEnqueue = true;
    bool disableMouseCapture = false;
    bool showMouseCursor = false;
    bool disableFpsLimitOnFocusLoss = false;
    int increaseDlcCount = 0x1000;
    bool fixDlcCostumeCrash = true;
    bool allowCustomDlcMultiuse = true;
    bool allowCustomDlcIfNotInParty = true;

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
                check_boolean("Reverie", "AssetFixes", assetFixes);
                check_boolean("Reverie", "FixBgmEnqueue", fixBgmEnqueue);
                check_boolean("Reverie", "DisableMouseCapture", disableMouseCapture);
                check_boolean("Reverie", "ShowMouseCursor", showMouseCursor);
                check_boolean("Reverie", "DisableFpsLimitOnFocusLoss", disableFpsLimitOnFocusLoss);
                check_integer("Reverie", "IncreaseDlcCount", increaseDlcCount);
                check_boolean("Reverie", "FixDlcCostumeCrash", fixDlcCostumeCrash);
                check_boolean("Reverie", "AllowCustomDlcMultiuse", allowCustomDlcMultiuse);
                check_boolean("Reverie", "AllowCustomDlcIfNotInParty", allowCustomDlcIfNotInParty);
            }
        }
    }

    SenLib::ModLoad::CreateModDirectory(baseDirUtf8);

    bool assetCreationSuccess = true;
    if (assetFixes) {
        assetCreationSuccess = SenLib::Sen5::CreateAssetPatchIfNeeded(logger, baseDirUtf8);
    }

    SenLib::ModLoad::LoadModP3As(logger,
                                 s_LoadedModsData,
                                 baseDirUtf8,
                                 assetFixes,
                                 "ReverieMod",
                                 false,
                                 [](const HyoutaUtils::Ini::IniFile& ini) { return; });

    SenLib::Sen5::PatchExecData patchExecData;
    patchExecData.Logger = &logger;
    patchExecData.TextRegion = static_cast<char*>(codeBase);
    patchExecData.Version = version;
    patchExecData.Codespace = newPage;
    patchExecData.CodespaceEnd = newPageEnd;

    SenLib::Sen5::InjectAtFFileOpen(patchExecData, &FFileOpenForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen5::InjectAtBattleScriptExists(patchExecData, &FFileExistsForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen5::InjectAtFileExists1(patchExecData, &FFileExistsForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen5::InjectAtFileExists2(patchExecData, &FFileExistsForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen5::InjectAtFFileGetFilesize(patchExecData, &FFileGetFilesizeForwarder);
    Align16CodePage(logger, patchExecData.Codespace);
    SenLib::Sen5::InjectAtOpenFSoundFile(patchExecData, &FSoundOpenForwarder);
    Align16CodePage(logger, patchExecData.Codespace);

    AddSenPatcherVersionToTitle(patchExecData, s_LoadedModsData, !assetCreationSuccess);
    Align16CodePage(logger, patchExecData.Codespace);

    if (fixBgmEnqueue) {
        PatchMusicQueueing(patchExecData);
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
    if (disableFpsLimitOnFocusLoss) {
        PatchDisableFpsLimitOnFocusLoss(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (increaseDlcCount >= 0) {
        PatchIncreaseDlcCount(patchExecData, static_cast<uint32_t>(increaseDlcCount));
    }
    if (fixDlcCostumeCrash) {
        PatchDlcCostumeCrash(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (allowCustomDlcMultiuse) {
        PatchCustomDlcMultiuse(patchExecData);
        Align16CodePage(logger, patchExecData.Codespace);
    }
    if (allowCustomDlcIfNotInParty) {
        PatchCustomDlcIfNotInParty(patchExecData);
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

PDirectSoundCreate8 InjectionDllInitializer() {
    HyoutaUtils::Logger logger(HyoutaUtils::IO::File(
        std::string_view("senpatcher_inject_reverie.log"), HyoutaUtils::IO::OpenMode::Write));
    logger.Log("Initializing Reverie hook from SenPatcher, version " SENPATCHER_VERSION "...\n");
    auto* forwarder = LoadForwarderAddress(logger);
    SetupHacks(logger);
    return forwarder;
}
static PDirectSoundCreate8 addr_PDirectSoundCreate8 = InjectionDllInitializer();

extern "C" {
HRESULT DirectSoundCreate8(void* lpcGuidDevice, void* ppDS8, void* pUnkOuter) {
    PDirectSoundCreate8 addr = addr_PDirectSoundCreate8;
    if (!addr) {
        return 0x8007000EL; // DSERR_OUTOFMEMORY
    }
    return addr(lpcGuidDevice, ppDS8, pUnkOuter);
}
}
