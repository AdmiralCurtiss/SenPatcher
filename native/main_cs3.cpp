#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

#include <array>
#include <cinttypes>
#include <cstdint>
#include <cstdio>

#include "crc32.h"

namespace {
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

static int64_t __fastcall CheckForModFile(void* ffile,
                                          const char* path,
                                          int unknownThirdParameter) {
    OutputDebugStringA("CheckForModFile called with path: ");
    OutputDebugStringA(path);
    OutputDebugStringA("\n");
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
                                                                : (0x1400f59fb - 0x140001000));


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

    // allocate extra page for code
    size_t newPageLength = 0x1000;
    char* newPage = static_cast<char*>(VirtualAlloc(nullptr, 0x1000, MEM_COMMIT, PAGE_READWRITE));
    if (!newPage) {
        logger.Log("VirtualAlloc failed, skipping remaining patches.\n");
        return nullptr;
    }
    char* newPageStart = newPage;
    char* newPageEnd = newPage + newPageLength;

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
