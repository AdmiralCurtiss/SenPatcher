#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchFixControllerMappings(SenPatcher::Logger& logger,
                                char* textRegion,
                                GameVersion version,
                                char*& codespace,
                                char* codespaceEnd) {
    using namespace SenPatcher::x64;
    using JC = JumpCondition;
    const bool jp = version == GameVersion::Japanese;

    char* addressStructMemAlloc =
        GetCodeAddressJpEn(version, textRegion, 0x14012dd89, 0x140131309) + 2;
    char* addressInjectPos = GetCodeAddressJpEn(version, textRegion, 0x14012d88c, 0x140130e0e);
    char* addressMapLookupCode = GetCodeAddressJpEn(version, textRegion, 0x14012e086, 0x140131622);
    size_t lengthMapLookupCodeForDelete = (jp ? 0x45 : 0x3f);
    char* addressMapLookupSuccessForDelete =
        GetCodeAddressJpEn(version, textRegion, 0x14012e0d8, 0x14013166e);
    size_t lengthMapLookupSuccessForDelete = (jp ? 4 : 3);
    char* jpSwapActions45 = textRegion + (0x14012d785u - 0x140001000u); // only in JP build
    static constexpr size_t jpSwapActions45Len = 13;                    // only in JP build
    assert(addressMapLookupCode < addressMapLookupSuccessForDelete);

    const auto write_instruction_48 = [&](char*& codepos, uint64_t instr) {
        *codepos++ = (char)((instr >> 40) & 0xff);
        *codepos++ = (char)((instr >> 32) & 0xff);
        *codepos++ = (char)((instr >> 24) & 0xff);
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_40 = [&](char*& codepos, uint64_t instr) {
        *codepos++ = (char)((instr >> 32) & 0xff);
        *codepos++ = (char)((instr >> 24) & 0xff);
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_32 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 24) & 0xff);
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_24 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_16 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };

    // increase struct allocation by 0x20 bytes
    uint8_t allocLengthOld;
    {
        PageUnprotect page(logger, addressStructMemAlloc, 1);
        allocLengthOld = *addressStructMemAlloc;
        uint8_t allocLengthNew = allocLengthOld + 0x20;
        *addressStructMemAlloc = allocLengthNew;
    }

    {
        BranchHelper1Byte begin_loop_1;
        BranchHelper1Byte begin_loop_2;
        BranchHelper1Byte lookup_1;
        BranchHelper1Byte lookup_2;
        BranchHelper1Byte lookup_3;
        BranchHelper1Byte lookup_4;
        BranchHelper1Byte lookup_5;
        BranchHelper1Byte lookup_fail;

        auto injectResult = InjectJumpIntoCode<12>(logger, addressInjectPos, R64::RAX, codespace);

        // initialize the lookup table so every button points at itself
        write_instruction_32(
            codespace, 0x4d8d6500u | ((uint32_t)allocLengthOld)); // lea r12,[r13+allocLengthOld]
        write_instruction_24(codespace, 0x4d33c9);                // xor r9,r9
        begin_loop_1.SetTarget(codespace);
        write_instruction_32(codespace, 0x4f8d3c0c); // lea r15,[r12+r9]
        write_instruction_24(codespace, 0x45880f);   // mov byte ptr[r15],r9b
        write_instruction_24(codespace, 0x49ffc1);   // inc r9
        write_instruction_32(codespace, 0x4983f920); // cmp r9,20h
        begin_loop_1.WriteJump(codespace, JC::JB);   // jb begin_loop_1

        // look up each key in the (presumably) std::map<int, int>
        // and write it into the lookup table in the other direction
        write_instruction_24(codespace, 0x4d33c9); // xor r9,r9
        begin_loop_2.SetTarget(codespace);
        write_instruction_24(codespace, 0x498bdd);   // mov rbx,r13
        write_instruction_24(codespace, 0x488b03);   // mov rax,qword ptr [rbx]
        write_instruction_32(codespace, 0x488b5010); // mov rdx,qword ptr [rax + 0x10]
        write_instruction_24(codespace, 0x488bc2);   // mov rax,rdx
        write_instruction_32(codespace, 0x488b4a08); // mov rcx,qword ptr [rdx + 0x8]
        write_instruction_32(codespace, 0x80791900); // cmp byte ptr [rcx + 0x19],0x0
        lookup_4.WriteJump(codespace, JC::JNZ);      // jnz lookup_4
        lookup_1.SetTarget(codespace);
        write_instruction_32(codespace, 0x4439491c); // cmp dword ptr [rcx + 0x1c],r9d
        lookup_2.WriteJump(codespace, JC::JGE);      // jge lookup_2
        write_instruction_32(codespace, 0x488b4910); // mov rcx,qword ptr [rcx + 0x10]
        lookup_3.WriteJump(codespace, JC::JMP);      // jmp lookup_3
        lookup_2.SetTarget(codespace);
        write_instruction_24(codespace, 0x488bc1); // mov rax,rcx
        write_instruction_24(codespace, 0x488b09); // mov rcx,qword ptr [rcx]
        lookup_3.SetTarget(codespace);
        write_instruction_32(codespace, 0x80791900); // cmp byte ptr [rcx + 0x19],0x0
        lookup_1.WriteJump(codespace, JC::JZ);       // jz  lookup_1
        write_instruction_24(codespace, 0x483bc2);   // cmp rax,rdx
        lookup_4.WriteJump(codespace, JC::JZ);       // jz  lookup_4
        write_instruction_32(codespace, 0x443b481c); // cmp r9d,dword ptr [rax + 0x1c]
        lookup_5.WriteJump(codespace, JC::JGE);      // jge lookup_5
        lookup_4.SetTarget(codespace);
        write_instruction_24(codespace, 0x488bc2); // mov rax,rdx
        lookup_5.SetTarget(codespace);
        write_instruction_24(codespace, 0x483bc2); // cmp rax,rdx
        write_instruction_24(codespace, 0x418bc9); // mov ecx,r9d
        lookup_fail.WriteJump(codespace, JC::JZ);  // jz  lookup_fail
        write_instruction_24(codespace, 0x8b4820); // mov ecx,dword ptr[rax + 20h]
        lookup_fail.SetTarget(codespace);
        // r9d now contains the UNMAPPED value
        // ecx now contains the MAPPED value
        write_instruction_32(codespace, 0x45880c0c); // mov byte ptr[r12+rcx],r9b
        write_instruction_24(codespace, 0x49ffc1);   // inc r9
        write_instruction_32(codespace, 0x4983f920); // cmp r9,20h
        begin_loop_2.WriteJump(codespace, JC::JB);   // jb begin_loop_2

        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        write_instruction_24(codespace, 0x498bc5); // mov rax,r13 ; restore clobbered rax

        Emit_MOV_R64_IMM64(
            codespace, R64::RSI, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RSI);
    }

    {
        PageUnprotect page(logger,
                           addressMapLookupCode,
                           (addressMapLookupSuccessForDelete - addressMapLookupCode)
                               + lengthMapLookupSuccessForDelete);

        // clear out old lookup code
        char* tmp = addressMapLookupCode;
        for (size_t i = 0; i < lengthMapLookupCodeForDelete; ++i) {
            *tmp++ = 0x90; // nop
        }
        tmp = addressMapLookupSuccessForDelete;
        for (size_t i = 0; i < lengthMapLookupSuccessForDelete; ++i) {
            *tmp++ = 0x90; // nop
        }

        // replace with new lookup code
        BranchHelper1Byte lookup_success;
        BranchHelper1Byte lookup_fail;

        // on success: result should be in ecx (on US) or eax (on JP)
        // on fail: result doesn't matter, restores itself
        lookup_success.SetTarget(addressMapLookupSuccessForDelete
                                 + lengthMapLookupSuccessForDelete);
        lookup_fail.SetTarget(addressMapLookupCode + lengthMapLookupCodeForDelete);

        tmp = addressMapLookupCode;
        if (!jp) {
            write_instruction_24(tmp, 0x418bc1); // mov eax,r9d
        }
        write_instruction_24(tmp, 0x83f820); // cmp eax,20h
        lookup_fail.WriteJump(tmp, JC::JNB); // jnb lookup_fail
        if (!jp) {
            write_instruction_32(tmp, 0x0fb64c03u); // movzx ecx,byte ptr[rbx+rax+allocLengthOld]
            *tmp++ = allocLengthOld;
            write_instruction_24(tmp, 0x83f920); // cmp ecx,20h
        } else {
            write_instruction_32(tmp, 0x0fb64403u); // movzx eax,byte ptr[rbx+rax+allocLengthOld]
            *tmp++ = allocLengthOld;
            write_instruction_24(tmp, 0x83f820); // cmp eax,20h
        }
        lookup_fail.WriteJump(tmp, JC::JNB);    // jnb lookup_fail
        lookup_success.WriteJump(tmp, JC::JMP); // jmp lookup_success
    }

    if (jp) {
        // swap actions 4/5 on controller config readin so we get identical mapping behavior between
        // JP/US builds
        auto injectResult =
            InjectJumpIntoCode<jpSwapActions45Len>(logger, jpSwapActions45, R64::RDX, codespace);
        const auto& ow = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, ow.data(), ow.size() - 5);
        codespace += ow.size() - 5;

        // the last 5 bytes we overwrote was a call instruction with a relative address, fix that
        int32_t relativeAddress;
        std::memcpy(&relativeAddress, ow.data() + ow.size() - 4, 4);
        char* absoluteAddress = injectResult.JumpBackAddress + relativeAddress;
        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(absoluteAddress));
        Emit_CALL_R64(codespace, R64::RAX); // call (replaced function call)

        BranchHelper1Byte check5;
        BranchHelper1Byte checkdone;
        write_instruction_40(codespace, 0x488d4c2450);   // lea rcx,[rsp+50h]
        write_instruction_16(codespace, 0x8b11);         // mov edx,dword ptr[rcx]
        write_instruction_24(codespace, 0x83fa04);       // cmp edx,4
        check5.WriteJump(codespace, JC::JNE);            // jne check5
        write_instruction_48(codespace, 0xc70105000000); // mov dword ptr[rcx],5
        write_instruction_16(codespace, 0xeb0b);         // jmp checkdone
        check5.SetTarget(codespace);
        write_instruction_24(codespace, 0x83fa05);       // cmp edx,5
        checkdone.WriteJump(codespace, JC::JNE);         // jne checkdone
        write_instruction_48(codespace, 0xc70104000000); // mov dword ptr[rcx],4
        checkdone.SetTarget(codespace);

        Emit_MOV_R64_IMM64(
            codespace, R64::RCX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RCX);
    }
}
} // namespace SenLib::Sen3
