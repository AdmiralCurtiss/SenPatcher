#include "pch.h"

#include <array>
#include <cinttypes>
#include <cstdio>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "fmt/format.h"

namespace {
struct PageUnprotect {
    void* Address;
    size_t Length;
    DWORD Attributes;

    PageUnprotect(void* addr, size_t length) {
        // FIXME: check length/alignment, this might span multiple pages!
        Length = 0x1000;
        Address = reinterpret_cast<void*>(reinterpret_cast<uint64_t>(addr) & (~(Length - 1)));
        VirtualProtect(Address, Length, PAGE_READWRITE, &Attributes);
    }

    ~PageUnprotect() {
        DWORD tmp;
        VirtualProtect(Address, Length, Attributes, &tmp);
    }
};
} // namespace

template<size_t S>
static std::array<char, S> ReadInstruction(char*& ptr) {
    std::array<char, S> data;
    memcpy(data.data(), ptr, S);
    ptr += S;
    return data;
}

static void WriteInstruction(const std::initializer_list<char>& data, char*& ptr) {
    memcpy(ptr, data.begin(), data.size());
    ptr += data.size();
}

using PDirectInput8Create = HRESULT(WINAPI*)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf,
                                             LPVOID* ppvOut, void* punkOuter);
static PDirectInput8Create LoadForwarderAddress() {
    constexpr int total = 2048;
    WCHAR tmp[total];
    UINT count = GetSystemDirectoryW(tmp, sizeof(tmp) / sizeof(TCHAR));
    if (count == 0 || count > total - 32) {
        return nullptr;
    }
    memcpy(tmp + count, L"\\dinput8.dll\0", sizeof(L"\\dinput8.dll\0"));

    HMODULE dll = ::LoadLibraryW(tmp);
    if (!dll) {
        return nullptr;
    }
    void* addr = ::GetProcAddress(dll, "DirectInput8Create");
    return (PDirectInput8Create)addr;
}

struct script_exec_data {
    void* vtableptr;
    uint32_t unknown0;
    uint32_t unknown1;
    char filename[0x20];
    char functionname[0x30];
    uint32_t unknown2;
    void* unknown3;
    char* file;
    uint32_t position;
    uint32_t unknown4;
    uint32_t unknown5;
    int32_t unknown6;
    uint32_t unknown7;
    uint32_t unknown8;
    uint32_t unknown9;
};

struct script_exec_local_data {
    uint32_t opcode;
    script_exec_data data;
};

std::recursive_mutex s_script_exec_mutex;
std::unordered_map<script_exec_data*, std::vector<script_exec_local_data>> s_script_exec_stacks;

// std::FILE* s_script_exec_collect_file = fopen("a.txt", "wb");

static void __fastcall log_script_execution(uint8_t type, uint32_t opcode, script_exec_data* data) {
    std::lock_guard<std::recursive_mutex> g(s_script_exec_mutex);
    // auto& local_data = s_script_exec_stacks[data].emplace_back();
    // local_data.opcode = opcode;
    // local_data.data = *data;

    // auto str = fmt::format("opcode start {0:02x}: File {1}, Function {2}\n", opcode,
    //                       local_data.data.filename, local_data.data.functionname);
    // OutputDebugStringA(str.c_str());

    // fputc(type, s_script_exec_collect_file);
    // fwrite(&opcode, sizeof(opcode), 1, s_script_exec_collect_file);
    // fwrite(data, sizeof(*data), 1, s_script_exec_collect_file);

    return;
}

static void __fastcall log_script_execution_end(uint8_t type, uint32_t opcode,
                                                script_exec_data* data) {
    std::lock_guard<std::recursive_mutex> g(s_script_exec_mutex);
    // auto stack_it = s_script_exec_stacks.find(data);
    // auto& stack = stack_it->second;
    // script_exec_local_data local_data = stack.back();
    // stack.pop_back();
    // if (stack.empty()) {
    //    s_script_exec_stacks.erase(stack_it);
    //}

    // auto str = fmt::format("opcode end   {0:02x}: File {1}, Function {2}; consumed {3} bytes\n",
    //                       opcode, local_data.data.filename, local_data.data.functionname,
    //                       (int32_t)(data->position - local_data.data.position));
    // OutputDebugStringA(str.c_str());

    // fputc(type, s_script_exec_collect_file);
    // fwrite(&opcode, sizeof(opcode), 1, s_script_exec_collect_file);
    // fwrite(data, sizeof(*data), 1, s_script_exec_collect_file);

    return;
}


static void __fastcall log_ed8_script_execution_start(uint32_t opcode, script_exec_data* data) {
    log_script_execution(0, opcode, data);
}
static void __fastcall log_ed8_script_execution_end(uint32_t opcode, script_exec_data* data) {
    log_script_execution_end(0 | 0x80, opcode, data);
}
static void __fastcall log_scenario_script_execution_start(uint32_t opcode,
                                                           script_exec_data* data) {
    log_script_execution(1, opcode, data);
}
static void __fastcall log_scenario_script_execution_end(uint32_t opcode, script_exec_data* data) {
    log_script_execution_end(1 | 0x80, opcode, data);
}
static void __fastcall log_ui_script_execution_start(uint32_t opcode, script_exec_data* data) {
    log_script_execution(2, opcode, data);
}
static void __fastcall log_ui_script_execution_end(uint32_t opcode, script_exec_data* data) {
    log_script_execution_end(2 | 0x80, opcode, data);
}

namespace {
struct lipflap_thisstruct {
    char* lipflapdata;
};
struct lipflap_stackstruct {
    uint32_t edx;
    uint32_t ecx;
    uint32_t unknown;
    uint32_t param1;
    uint32_t param2;
};
} // namespace

static void __fastcall log_lipflaps(lipflap_thisstruct* thisptr, lipflap_stackstruct* stackptr) {
    std::string s = fmt::format(
        "edx {0:08x}, ecx {1:08x}, unknown {2:08x}, param1 {3:08}, param2 {4:08}\n", stackptr->edx,
        stackptr->ecx, stackptr->unknown, stackptr->param1, stackptr->param2);
    OutputDebugStringA(s.c_str());
}

static void inject_script_exec(char*& current_free_offset, void* inject_loc, void* startfunc,
                               void* endfunc) {
    {
        char* exec_script_single_action_func = reinterpret_cast<char*>(inject_loc);
        size_t exec_script_single_action_size = 0x20;

        char* entry_point = current_free_offset;
        WriteInstruction({(char)0x58}, current_free_offset); // pop eax (see bottom for context)

        std::array<char, 3> push_ebp_mov_ebp_esp;
        memcpy(push_ebp_mov_ebp_esp.data(), exec_script_single_action_func, 3);

        WriteInstruction({(char)0x50}, current_free_offset); // push eax
        WriteInstruction({(char)0x51}, current_free_offset); // push ecx
        WriteInstruction({(char)0x52}, current_free_offset); // push edx

        WriteInstruction({(char)0x8b, (char)0x4d, (char)0x0c},
                         current_free_offset); // mov ecx,dword ptr[ebp+0xc]
        WriteInstruction({(char)0x8b, (char)0x55, (char)0x08},
                         current_free_offset); // mov edx,dword ptr[ebp+0x8]

        // mov eax,addr
        WriteInstruction({(char)0xb8}, current_free_offset);
        uint32_t addr = reinterpret_cast<uint32_t>(startfunc);
        memcpy(current_free_offset, &addr, 4);
        current_free_offset += 4;

        WriteInstruction({(char)0xff, (char)0xd0}, current_free_offset); // call eax
        WriteInstruction({(char)0x5a}, current_free_offset);             // pop edx
        WriteInstruction({(char)0x59}, current_free_offset);             // pop ecx
        WriteInstruction({(char)0x58}, current_free_offset);             // pop eax

        memcpy(current_free_offset, exec_script_single_action_func + 3,
               exec_script_single_action_size - (3 + 5));
        current_free_offset += (exec_script_single_action_size - (3 + 5));

        WriteInstruction({(char)0x50}, current_free_offset); // push eax
        WriteInstruction({(char)0x51}, current_free_offset); // push ecx
        WriteInstruction({(char)0x52}, current_free_offset); // push edx

        WriteInstruction({(char)0x8b, (char)0x4d, (char)0x0c},
                         current_free_offset); // mov ecx,dword ptr[ebp+0xc]
        WriteInstruction({(char)0x8b, (char)0x55, (char)0x08},
                         current_free_offset); // mov edx,dword ptr[ebp+0x8]

        // mov eax,addr
        WriteInstruction({(char)0xb8}, current_free_offset);
        addr = reinterpret_cast<uint32_t>(endfunc);
        memcpy(current_free_offset, &addr, 4);
        current_free_offset += 4;

        WriteInstruction({(char)0xff, (char)0xd0}, current_free_offset); // call eax
        WriteInstruction({(char)0x5a}, current_free_offset);             // pop edx
        WriteInstruction({(char)0x59}, current_free_offset);             // pop ecx
        WriteInstruction({(char)0x58}, current_free_offset);             // pop eax

        memcpy(current_free_offset,
               exec_script_single_action_func + (exec_script_single_action_size - 5), 5);
        current_free_offset += 5;

        {
            PageUnprotect unprotect(exec_script_single_action_func, exec_script_single_action_size);
            memset(exec_script_single_action_func, 0xcc, exec_script_single_action_size);

            char* func_replace = reinterpret_cast<char*>(exec_script_single_action_func);

            memcpy(func_replace, push_ebp_mov_ebp_esp.data(), 3);
            func_replace += 3;

            WriteInstruction({(char)0x50}, func_replace); // push eax

            // mov eax,addr2
            WriteInstruction({(char)0xb8}, func_replace);
            uint32_t addr2 = reinterpret_cast<uint32_t>(entry_point);
            memcpy(func_replace, &addr2, 4);
            func_replace += 4;

            WriteInstruction({(char)0xff, (char)0xe0}, func_replace); // jmp eax
        }
    }
}

static void inject_lipflap_monitor(char*& current_free_offset, void* func) {
    // the function at 0x5780e0 is called for lipflap info
    // this has no outgoing function calls, so we can completely copy & paste it elsewhere and
    // hijack the function itself. we also know eax is safe to change here.

    void* inject_func = reinterpret_cast<void*>(0x5780e0);
    size_t inject_func_len = 0x578134 - 0x5780e0;

    PageUnprotect unprotect(inject_func, inject_func_len);
    void* copied_function = current_free_offset;
    memcpy(current_free_offset, inject_func, inject_func_len);
    current_free_offset += inject_func_len;
    memset(inject_func, 0xcc, inject_func_len);

    char* inject_func_replacement = reinterpret_cast<char*>(inject_func);
    WriteInstruction({(char)0x51}, inject_func_replacement);             // push ecx
    WriteInstruction({(char)0x52}, inject_func_replacement);             // push edx
    WriteInstruction({(char)0x89, (char)0xe2}, inject_func_replacement); // mov edx,esp

    // mov eax,func
    WriteInstruction({(char)0xb8}, inject_func_replacement);
    uint32_t addr1 = reinterpret_cast<uint32_t>(func);
    memcpy(inject_func_replacement, &addr1, 4);
    inject_func_replacement += 4;

    WriteInstruction({(char)0xff, (char)0xd0}, inject_func_replacement); // call eax

    WriteInstruction({(char)0x5a}, inject_func_replacement); // pop edx
    WriteInstruction({(char)0x59}, inject_func_replacement); // pop ecx

    // mov eax,copied_function
    WriteInstruction({(char)0xb8}, inject_func_replacement);
    uint32_t addr2 = reinterpret_cast<uint32_t>(copied_function);
    memcpy(inject_func_replacement, &addr2, 4);
    inject_func_replacement += 4;

    WriteInstruction({(char)0xff, (char)0xe0}, inject_func_replacement); // jmp eax
}

static PDirectInput8Create addr_PDirectInput8Create = 0;
static void* SetupHacks() {
    addr_PDirectInput8Create = LoadForwarderAddress();

    void* new_page = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT, PAGE_READWRITE);
    if (!new_page) {
        return nullptr;
    }

    char* current_free_offset = reinterpret_cast<char*>(new_page);
    inject_lipflap_monitor(current_free_offset, log_lipflaps);
    // inject_script_exec(current_free_offset, reinterpret_cast<void*>(0x005e0e20),
    //                    log_ed8_script_execution_start, log_ed8_script_execution_end);
    // inject_script_exec(current_free_offset, reinterpret_cast<void*>(0x005e0e40),
    //                    log_scenario_script_execution_start, log_scenario_script_execution_end);
    // inject_script_exec(current_free_offset, reinterpret_cast<void*>(0x00632790),
    //                    log_ui_script_execution_start, log_ui_script_execution_end);

    // mark newly allocated page as executable
    {
        DWORD tmpdword;
        VirtualProtect(new_page, 0x1000, PAGE_EXECUTE_READ, &tmpdword);
        FlushInstructionCache(GetCurrentProcess(), new_page, 0x1000);
    }

    return new_page;
}
static void* dummy = SetupHacks();

extern "C" {
HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut,
                                  void* punkOuter) {
    PDirectInput8Create addr = addr_PDirectInput8Create;
    if (!addr) {
        return 0x8007000EL; // DIERR_OUTOFMEMORY
    }
    return addr(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}
}