#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void FixInGameButtonMappingValidity(SenPatcher::Logger& logger,
                                    char* textRegion,
                                    GameVersion version,
                                    char*& codespace,
                                    char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* entryPoint = GetCodeAddressJpEn(version, textRegion, 0x140447427, 0x140453257);
    size_t length = 0x44;
    PageUnprotect page(logger, entryPoint, length);

    // remove buggy code
    {
        char* tmp = entryPoint;
        for (size_t i = 0; i < length; ++i) {
            *tmp++ = 0xcc; // int 3
        }
    }

    BranchHelper1Byte done;
    done.SetTarget(entryPoint + length);

    // input:
    //   ax as the button, same order as in the config menu (0-9)
    //   bx as the action, same order as in the config menu (0-12)
    //   rax, rcx, rdx may be clobbered
    // output:
    //   esi = 0 if option should be enabled, esi = 3 if option should be disabled

    char* inject = entryPoint;
    const auto write_instruction_32 = [&](uint32_t instr) {
        *inject++ = (char)((instr >> 24) & 0xff);
        *inject++ = (char)((instr >> 16) & 0xff);
        *inject++ = (char)((instr >> 8) & 0xff);
        *inject++ = (char)(instr & 0xff);
    };
    const auto write_instruction_24 = [&](uint32_t instr) {
        *inject++ = (char)((instr >> 16) & 0xff);
        *inject++ = (char)((instr >> 8) & 0xff);
        *inject++ = (char)(instr & 0xff);
    };
    const auto write_integer_16 = [&](uint32_t value) {
        *inject++ = (char)(value & 0xff);
        *inject++ = (char)((value >> 8) & 0xff);
    };

    write_instruction_24(0x488d0d); // lea rcx,[lookup_table]
    write_instruction_32(0);        // ^ relative address of lookup table
    char* lookup_table_inject_address = inject;
    write_instruction_32(0x480fb7c0);           // movzx rax,ax
    write_instruction_32(0x488d0c41);           // lea rcx,[rcx+rax*2]
    write_instruction_24(0x668b01);             // mov ax,word ptr[rcx]
    write_instruction_24(0x668bcb);             // mov cx,bx
    write_instruction_24(0x66d3e8);             // shr ax,cl
    write_instruction_32(0x6683e001);           // and ax,1
    write_instruction_24(0x668bc8);             // mov cx,ax
    write_instruction_24(0x66d1e0);             // shl ax,1
    write_instruction_24(0x660bc1);             // or ax,cx
    write_instruction_24(0x0fb7f0);             // movzx esi,ax
    done.WriteJump(inject, JumpCondition::JMP); // jmp done

    while (std::bit_cast<uint64_t>(inject) % 4 != 0) {
        *inject++ = 0xcc; // int 3
    }

    char* lookup_table_address = inject;
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad Up
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad Left
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad Right
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad L1
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad R1
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad R3
    write_integer_16(0x0c42); // bitmask for disallowed options for D-Pad Circle
    write_integer_16(0x0c00); // bitmask for disallowed options for D-Pad Select
    write_integer_16(0x03ff); // bitmask for disallowed options for D-Pad Select (Battle)
    write_integer_16(0x03ff); // bitmask for disallowed options for D-Pad Start (Battle)

    const uint32_t relativeOffset =
        static_cast<uint32_t>(lookup_table_address - lookup_table_inject_address);
    std::memcpy(lookup_table_inject_address - 4, &relativeOffset, 4);
}
} // namespace SenLib::Sen3