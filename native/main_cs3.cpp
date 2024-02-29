#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

#include <array>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <vector>

#include "lz4/lz4.h"
#include "zstd/zdict.h"
#include "zstd/zstd.h"

#include "crc32.h"

#include "file.h"

#include "p3a/p3a.h"
#include "p3a/structs.h"

namespace {
struct ZSTD_DCtx_Deleter {
    void operator()(ZSTD_DCtx* ptr) {
        if (ptr) {
            ZSTD_freeDCtx(ptr);
        }
    }
};
using ZSTD_DCtx_UniquePtr = std::unique_ptr<ZSTD_DCtx, ZSTD_DCtx_Deleter>;

enum class GameVersion {
    Unknown,
    English,
    Japanese,
};

struct Logger {
    FILE* f;

    Logger(const char* filename) {
        f = fopen(filename, "w");
    }

    ~Logger() {
        if (f) {
            fclose(f);
        }
    }

    Logger& Log(const char* text) {
        if (f) {
            fwrite(text, strlen(text), 1, f);
            fflush(f);
        }
        return *this;
    }

    Logger& LogPtr(const void* ptr) {
        if (f) {
            char buffer[32];
            int len = sprintf(buffer, "0x%016" PRIxPTR, reinterpret_cast<uintptr_t>(ptr));
            fwrite(buffer, len, 1, f);
            fflush(f);
        }
        return *this;
    }

    Logger& LogInt(int v) {
        if (f) {
            char buffer[32];
            int len = sprintf(buffer, "%d", v);
            fwrite(buffer, len, 1, f);
            fflush(f);
        }
        return *this;
    }

    Logger& LogHex(unsigned long long v) {
        if (f) {
            char buffer[32];
            int len = sprintf(buffer, "0x%llx", v);
            fwrite(buffer, len, 1, f);
            fflush(f);
        }
        return *this;
    }

    Logger& LogFloat(float v) {
        if (f) {
            char buffer[32];
            int len = sprintf(buffer, "%g", v);
            fwrite(buffer, len, 1, f);
            fflush(f);
        }
        return *this;
    }
};

struct PageUnprotect {
    Logger& Log;
    void* Address;
    size_t Length;
    DWORD Attributes;

    PageUnprotect(Logger& logger, void* addr, size_t length) : Log(logger) {
        // FIXME: check length/alignment, this might span multiple pages!
        Length = 0x1000;
        Address = reinterpret_cast<void*>(reinterpret_cast<uint64_t>(addr) & (~(Length - 1)));
        Log.Log("Unprotecting ").LogHex(Length).Log(" bytes at ").LogPtr(Address);
        if (VirtualProtect(Address, Length, PAGE_READWRITE, &Attributes)) {
            Log.Log(" -> Success, previous attributes were ").LogHex(Attributes).Log(".\n");
        } else {
            Log.Log(" -> Failed.\n");
        }
    }

    ~PageUnprotect() {
        DWORD tmp;
        Log.Log("Reprotecting ").LogHex(Length).Log(" bytes at ").LogPtr(Address);
        Log.Log(" to attributes ").LogHex(Attributes);
        if (VirtualProtect(Address, Length, Attributes, &tmp)) {
            Log.Log(" -> Success.\n");
        } else {
            Log.Log(" -> Failed.\n");
        }
    }
};
} // namespace

static int SelectOffset(GameVersion version, int en, int jp) {
    switch (version) {
        case GameVersion::English: return en;
        case GameVersion::Japanese: return jp;
        default: return 0;
    }
}

using PDirectInput8Create =
    HRESULT (*)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, void* punkOuter);
static PDirectInput8Create LoadForwarderAddress(Logger& logger) {
    constexpr int total = 5000;
    WCHAR tmp[total];
    UINT count = GetSystemDirectoryW(tmp, sizeof(tmp) / sizeof(WCHAR));
    if (count == 0 || count > total - 16) {
        logger.Log("Failed constructing path for system dinput8.dll.\n");
        return nullptr;
    }
    memcpy(tmp + count, L"\\dinput8.dll\0", sizeof(L"\\dinput8.dll\0"));

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

static void WriteFloat(Logger& logger, void* addr, float value) {
    logger.Log("Writing float ").LogFloat(value).Log(" to ").LogPtr(addr).Log(".\n");
    PageUnprotect unprotect(logger, addr, 4);
    memcpy(addr, &value, 4);
}

static void WriteInt(Logger& logger, void* addr, int value) {
    logger.Log("Writing int ").LogInt(value).Log(" to ").LogPtr(addr).Log(".\n");
    PageUnprotect unprotect(logger, addr, 4);
    memcpy(addr, &value, 4);
}

static void WriteByte(Logger& logger, void* addr, char value) {
    logger.Log("Writing byte ").LogHex(value).Log(" to ").LogPtr(addr).Log(".\n");
    PageUnprotect unprotect(logger, addr, 1);
    memcpy(addr, &value, 1);
}

static char* Align16CodePage(Logger& logger, void* new_page) {
    logger.Log("Aligning ").LogPtr(new_page).Log(" to 16 bytes.\n");
    char* p = reinterpret_cast<char*>(new_page);
    *p++ = 0xcc;
    while ((reinterpret_cast<unsigned long long>(p) & 0xf) != 0) {
        *p++ = 0xcc;
    }
    return p;
}

static GameVersion FindImageBase(Logger& logger, void** code) {
    GameVersion gameVersion = GameVersion::Unknown;
    MEMORY_BASIC_INFORMATION info;
    memset(&info, 0, sizeof(info));
    *code = nullptr;
    for (unsigned long long address = 0; address < 0x80000000000; address += info.RegionSize) {
        if (VirtualQuery(reinterpret_cast<void*>(address), &info, sizeof(info)) == 0) {
            break;
        }

        if (info.State == MEM_COMMIT && info.Type == MEM_IMAGE) {
            logger.Log("Allocation at ")
                .LogPtr(info.AllocationBase)
                .Log(", base ptr ")
                .LogPtr(info.BaseAddress)
                .Log(", size ")
                .LogHex(info.RegionSize)
                .Log(", protection ")
                .LogHex(info.Protect)
                .Log(".\n");
            if ((*code == 0) && info.RegionSize == 0x7e9000 && info.Protect == PAGE_EXECUTE_READ) {
                // crc_t crc = crc_init();
                // crc = crc_update(crc, info.BaseAddress, info.RegionSize);
                // crc = crc_finalize(crc);
                // logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                logger.Log("Appears to be the EN version.\n");
                *code = info.BaseAddress;
                gameVersion = GameVersion::English;
            } else if ((*code == 0) && info.RegionSize == 0x7dc000
                       && info.Protect == PAGE_EXECUTE_READ) {
                // crc_t crc = crc_init();
                // crc = crc_update(crc, info.BaseAddress, info.RegionSize);
                // crc = crc_finalize(crc);
                // logger.Log("Checksum is ").LogHex(crc).Log(".\n");
                logger.Log("Appears to be the JP version.\n");
                *code = info.BaseAddress;
                gameVersion = GameVersion::Japanese;
            }

            // logger.Log("First 64 bytes are:");
            // for (int i = 0; i < (info.RegionSize < 64 ? info.RegionSize : 64); ++i) {
            //    logger.Log(" ").LogHex(*(reinterpret_cast<unsigned char*>(info.BaseAddress) + i));
            // }
            // logger.Log("\n");
        }
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
    // should be allocated with the tracking malloc at 0x1405e03f0 (size, 4, 0, 0, 0)
    void* MemoryPointerForFreeing;

    uint32_t MemoryPosition;
};

using PTrackedMalloc = void*(__fastcall*)(uint64_t size,
                                          uint64_t alignment,
                                          const char* file,
                                          uint64_t line,
                                          uint64_t unknown);
using PTrackedFree = void(__fastcall*)(void* memory);

struct P3AFileRef {
    SenPatcher::P3A* Archive;
    SenPatcher::P3AFileInfo* FileInfo;
};

static PTrackedMalloc s_TrackedMalloc = nullptr;
static PTrackedFree s_TrackedFree = nullptr;
static bool s_CheckDevFolderForAssets = false;
static std::unique_ptr<SenPatcher::P3A[]> s_P3As;
static size_t s_CombinedFileInfoCount = 0;
static std::unique_ptr<P3AFileRef[]> s_CombinedFileInfos;

static bool Matches(const char* path, const char8_t* p3afile, size_t maxlen) {
    size_t i = 0;
    for (; i < maxlen; ++i) {
        // TODO: successive path separators in 'path' must be collapsed to one

        const char unfiltered = path[i];
        if (unfiltered == '\0')
            return p3afile[i] == '\0';
        const char filtered = unfiltered == '\\' ? '/'
                                                 : ((unfiltered >= 'A' && unfiltered <= 'Z')
                                                        ? (unfiltered + ('a' - 'A'))
                                                        : unfiltered);
        if (filtered != p3afile[i]) {
            return false;
        }
    }
    return path[maxlen] == '\0';
}

static void LoadModP3As() {
    s_CombinedFileInfoCount = 0;
    s_CombinedFileInfos.reset();
    s_P3As.reset();

    size_t p3acount = 0;
    std::unique_ptr<SenPatcher::P3A[]> p3as;
    {
        std::vector<SenPatcher::P3A> p3avector;
        std::error_code ec;
        std::filesystem::directory_iterator iterator(L"../../mods", ec);
        if (ec) {
            return;
        }
        for (auto const& entry : iterator) {
            if (entry.is_directory()) {
                continue;
            }

            SenPatcher::P3A& p3a = p3avector.emplace_back();
            if (!p3a.Load(entry.path())) {
                p3avector.pop_back();
            }
        }

        p3acount = p3avector.size();
        p3as = std::make_unique<SenPatcher::P3A[]>(p3acount);
        for (size_t i = 0; i < p3acount; ++i) {
            p3as[i] = std::move(p3avector[i]);
        }
    }

    size_t totalFileInfoCount = 0;
    for (size_t i = 0; i < p3acount; ++i) {
        totalFileInfoCount += p3as[i].FileCount;
    }

    std::unique_ptr<P3AFileRef[]> combinedFileInfos =
        std::make_unique<P3AFileRef[]>(totalFileInfoCount);
    size_t index = 0;
    for (size_t i = 0; i < p3acount; ++i) {
        auto& p3a = p3as[i];
        const size_t localFileInfoCount = p3a.FileCount;
        for (size_t j = 0; j < localFileInfoCount; ++j) {
            auto& fileinfo = p3a.FileInfo[j];
            combinedFileInfos[index].Archive = &p3a;
            combinedFileInfos[index].FileInfo = &fileinfo;

            // pre-filter filenames so they're all lowercase and use forward slashes
            for (size_t k = 0; k < fileinfo.Filename.size(); ++k) {
                const char c = fileinfo.Filename[k];
                if (c >= 'A' && c <= 'Z') {
                    fileinfo.Filename[k] = c + ('a' - 'A');
                } else if (c == '\\') {
                    fileinfo.Filename[k] = '/';
                }
            }

            ++index;
        }
    }

    s_P3As = std::move(p3as);
    s_CombinedFileInfoCount = totalFileInfoCount;
    s_CombinedFileInfos = std::move(combinedFileInfos);

    return;
}

static int64_t __fastcall CheckForModFile(FFile* ffile,
                                          const char* path,
                                          int unknownThirdParameter) {
    OutputDebugStringA("CheckForModFile called with path: ");
    OutputDebugStringA(path);
    OutputDebugStringA("\n");

    const size_t count = s_CombinedFileInfoCount;
    const P3AFileRef* const infos = s_CombinedFileInfos.get();
    for (size_t i = 0; i < count; ++i) {
        const P3AFileRef& ref = infos[i];
        const SenPatcher::P3AFileInfo& fi = *ref.FileInfo;
        if (Matches(path, fi.Filename.data(), fi.Filename.size())) {
            if (fi.UncompressedSize >= 0x8000'0000) {
                return 0;
            }

            switch (fi.CompressionType) {
                case SenPatcher::P3ACompressionType::None: {
                    auto& file = ref.Archive->FileHandle;
                    if (!file.SetPosition(fi.Offset)) {
                        return 0;
                    }

                    void* memory = s_TrackedMalloc(fi.UncompressedSize, 8, nullptr, 0, 0);
                    if (!memory) {
                        return 0;
                    }
                    if (file.Read(memory, fi.UncompressedSize) != fi.UncompressedSize) {
                        s_TrackedFree(memory);
                        return 0;
                    }
                    // TODO: check hash
                    ffile->Filesize = static_cast<uint32_t>(fi.UncompressedSize);
                    ffile->MemoryPointer = memory;
                    ffile->MemoryPointerForFreeing = memory;
                    ffile->MemoryPosition = 0;

                    OutputDebugStringA("  --> rerouted to PKA (uncompressed)\n");

                    return 1;
                }
                case SenPatcher::P3ACompressionType::LZ4: {
                    auto& file = ref.Archive->FileHandle;
                    if (!file.SetPosition(fi.Offset)) {
                        return 0;
                    }

                    void* memory = s_TrackedMalloc(fi.UncompressedSize, 8, nullptr, 0, 0);
                    if (!memory) {
                        return 0;
                    }
                    auto compressedMemory =
                        std::make_unique_for_overwrite<char[]>(fi.CompressedSize);
                    if (!compressedMemory) {
                        s_TrackedFree(memory);
                        return 0;
                    }
                    if (file.Read(compressedMemory.get(), fi.CompressedSize) != fi.CompressedSize) {
                        compressedMemory.reset();
                        s_TrackedFree(memory);
                        return 0;
                    }
                    // TODO: check hash
                    if (LZ4_decompress_safe(compressedMemory.get(),
                                            static_cast<char*>(memory),
                                            fi.CompressedSize,
                                            fi.UncompressedSize)
                        != fi.UncompressedSize) {
                        compressedMemory.reset();
                        s_TrackedFree(memory);
                        return 0;
                    }
                    compressedMemory.reset();

                    ffile->Filesize = static_cast<uint32_t>(fi.UncompressedSize);
                    ffile->MemoryPointer = memory;
                    ffile->MemoryPointerForFreeing = memory;
                    ffile->MemoryPosition = 0;

                    OutputDebugStringA("  --> rerouted to PKA (lz4)\n");

                    return 1;
                }
                case SenPatcher::P3ACompressionType::ZSTD: {
                    auto& file = ref.Archive->FileHandle;
                    if (!file.SetPosition(fi.Offset)) {
                        return 0;
                    }

                    void* memory = s_TrackedMalloc(fi.UncompressedSize, 8, nullptr, 0, 0);
                    if (!memory) {
                        return 0;
                    }
                    auto compressedMemory =
                        std::make_unique_for_overwrite<char[]>(fi.CompressedSize);
                    if (!compressedMemory) {
                        s_TrackedFree(memory);
                        return 0;
                    }
                    if (file.Read(compressedMemory.get(), fi.CompressedSize) != fi.CompressedSize) {
                        compressedMemory.reset();
                        s_TrackedFree(memory);
                        return 0;
                    }
                    // TODO: check hash
                    if (ZSTD_decompress(static_cast<char*>(memory),
                                        fi.UncompressedSize,
                                        compressedMemory.get(),
                                        fi.CompressedSize)
                        != fi.UncompressedSize) {
                        compressedMemory.reset();
                        s_TrackedFree(memory);
                        return 0;
                    }
                    compressedMemory.reset();

                    ffile->Filesize = static_cast<uint32_t>(fi.UncompressedSize);
                    ffile->MemoryPointer = memory;
                    ffile->MemoryPointerForFreeing = memory;
                    ffile->MemoryPosition = 0;

                    OutputDebugStringA("  --> rerouted to PKA (zstd)\n");

                    return 1;
                }
                case SenPatcher::P3ACompressionType::ZSTD_DICT: {
                    if (!ref.Archive->Dict) {
                        return 0;
                    }
                    auto& file = ref.Archive->FileHandle;
                    if (!file.SetPosition(fi.Offset)) {
                        return 0;
                    }

                    void* memory = s_TrackedMalloc(fi.UncompressedSize, 8, nullptr, 0, 0);
                    if (!memory) {
                        return 0;
                    }
                    auto compressedMemory =
                        std::make_unique_for_overwrite<char[]>(fi.CompressedSize);
                    if (!compressedMemory) {
                        s_TrackedFree(memory);
                        return 0;
                    }
                    ZSTD_DCtx_UniquePtr dctx = ZSTD_DCtx_UniquePtr(ZSTD_createDCtx());
                    if (!dctx) {
                        s_TrackedFree(memory);
                        return 0;
                    }
                    if (file.Read(compressedMemory.get(), fi.CompressedSize) != fi.CompressedSize) {
                        dctx.reset();
                        compressedMemory.reset();
                        s_TrackedFree(memory);
                        return 0;
                    }
                    // TODO: check hash
                    if (ZSTD_decompress_usingDDict(dctx.get(),
                                                   static_cast<char*>(memory),
                                                   fi.UncompressedSize,
                                                   compressedMemory.get(),
                                                   fi.CompressedSize,
                                                   ref.Archive->Dict)
                        != fi.UncompressedSize) {
                        dctx.reset();
                        compressedMemory.reset();
                        s_TrackedFree(memory);
                        return 0;
                    }
                    dctx.reset();
                    compressedMemory.reset();

                    ffile->Filesize = static_cast<uint32_t>(fi.UncompressedSize);
                    ffile->MemoryPointer = memory;
                    ffile->MemoryPointerForFreeing = memory;
                    ffile->MemoryPosition = 0;

                    OutputDebugStringA("  --> rerouted to PKA (zstd dict)\n");

                    return 1;
                }
                default: return 0;
            }
        }
    }

    if (s_CheckDevFolderForAssets) {
        std::u8string tmp = u8"dev/";
        tmp += (char8_t*)path;

        SenPatcher::IO::File file(std::filesystem::path(tmp), SenPatcher::IO::OpenMode::Read);
        if (file.IsOpen()) {
            auto length = file.GetLength();
            if (length && *length < 0x8000'0000) {
                ffile->NativeFileHandle = file.ReleaseHandle();
                ffile->Filesize = static_cast<uint32_t>(*length);

                OutputDebugStringA("  --> rerouted to ");
                OutputDebugStringA((char*)tmp.c_str());
                OutputDebugStringA("\n");

                return 1;
            }
        }
    }

    return 0;
}

static void InjectAtFFileOpen(Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd) {
    char* const entryPoint = textRegion
                             + (version == GameVersion::Japanese ? (0x1400f5270 - 0x140001000)
                                                                 : (0x1400f58f0 - 0x140001000));
    char* const exitPoint = textRegion
                            + (version == GameVersion::Japanese ? (0x1400f538d - 0x140001000)
                                                                : (0x1400f5a0d - 0x140001000));


    char* codespaceBegin = codespace;
    char* inject = entryPoint;
    std::array<char, 12> overwrittenInstructions;

    {
        PageUnprotect page(logger, inject, overwrittenInstructions.size());
        std::memcpy(overwrittenInstructions.data(), inject, overwrittenInstructions.size());

        // mov rax,codespaceBegin
        *inject++ = 0x48;
        *inject++ = 0xb8;
        std::memcpy(inject, &codespaceBegin, 8);
        inject += 8;
        // jmp rax
        *inject++ = 0xff;
        *inject++ = 0xe0;
    }

    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    // push rax
    *codespace++ = 0x50;
    // push rcx
    *codespace++ = 0x51;
    // push rdx
    *codespace++ = 0x52;
    // push r8
    *codespace++ = 0x41;
    *codespace++ = 0x50;

    // for some reason I cannot figure out, the caller has to reserve stack space for the called
    // function's parameters in x64??? sub rsp,0x20
    *codespace++ = 0x48;
    *codespace++ = 0x83;
    *codespace++ = 0xec;
    *codespace++ = 0x20;

    // mov rax,CheckForModFile
    *codespace++ = 0x48;
    *codespace++ = 0xb8;
    void* addr = &CheckForModFile;
    std::memcpy(codespace, &addr, 8);
    codespace += 8;
    // call rax
    *codespace++ = 0xff;
    *codespace++ = 0xd0;

    // add rsp,0x20
    *codespace++ = 0x48;
    *codespace++ = 0x83;
    *codespace++ = 0xc4;
    *codespace++ = 0x20;

    // test rax,rax
    *codespace++ = 0x48;
    *codespace++ = 0x85;
    *codespace++ = 0xc0;
    // pop r8
    *codespace++ = 0x41;
    *codespace++ = 0x58;
    // pop rdx
    *codespace++ = 0x5a;
    // pop rcx
    *codespace++ = 0x59;
    // pop rax
    *codespace++ = 0x58;

    // if successful, skip the following jmp r9
    // jnz +13
    *codespace++ = 0x75;
    *codespace++ = 0x0d;

    // go back to function and pretend nothing happened
    // mov r9,injectEnd
    *codespace++ = 0x49;
    *codespace++ = 0xb9;
    std::memcpy(codespace, &inject, 8);
    codespace += 8;
    // jmp r9
    *codespace++ = 0x41;
    *codespace++ = 0xff;
    *codespace++ = 0xe1;

    // return that we succeeded
    // mov eax,1
    *codespace++ = 0xb8;
    *codespace++ = 0x01;
    *codespace++ = 0x00;
    *codespace++ = 0x00;
    *codespace++ = 0x00;

    // jump to exit point
    // mov rcx,exitPoint
    *codespace++ = 0x48;
    *codespace++ = 0xb9;
    std::memcpy(codespace, &exitPoint, 8);
    codespace += 8;
    // jmp rcx
    *codespace++ = 0xff;
    *codespace++ = 0xe1;
}


static PDirectInput8Create addr_PDirectInput8Create = 0;
static void* SetupHacks() {
    Logger logger("senpatcher_inject_cs3.log");

    addr_PDirectInput8Create = LoadForwarderAddress(logger);

    void* codeBase = nullptr;
    GameVersion version = FindImageBase(logger, &codeBase);
    if (version == GameVersion::Unknown || !codeBase) {
        logger.Log("Failed finding executable in memory -- wrong game or version?\n");
        return nullptr;
    }

    s_TrackedMalloc = reinterpret_cast<PTrackedMalloc>(static_cast<char*>(codeBase)
                                                       + (version == GameVersion::Japanese
                                                              ? (0x1405d3fa0 - 0x140001000)
                                                              : (0x1405e03f0 - 0x140001000)));
    s_TrackedFree = reinterpret_cast<PTrackedFree>(static_cast<char*>(codeBase)
                                                   + (version == GameVersion::Japanese
                                                          ? (0x1405d3ed0 - 0x140001000)
                                                          : (0x1405e0320 - 0x140001000)));

    // allocate extra page for code
    size_t newPageLength = 0x1000;
    char* newPage = static_cast<char*>(VirtualAlloc(nullptr, 0x1000, MEM_COMMIT, PAGE_READWRITE));
    if (!newPage) {
        logger.Log("VirtualAlloc failed, skipping remaining patches.\n");
        return nullptr;
    }
    char* newPageStart = newPage;
    char* newPageEnd = newPage + newPageLength;

    LoadModP3As();

    InjectAtFFileOpen(logger, static_cast<char*>(codeBase), version, newPage, newPageEnd);

    // mark newly allocated page as executable
    {
        DWORD tmpdword;
        VirtualProtect(newPageStart, 0x1000, PAGE_EXECUTE_READ, &tmpdword);
        FlushInstructionCache(GetCurrentProcess(), newPageStart, 0x1000);
    }

    return newPageStart;
}
static void* dummy = SetupHacks();

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
