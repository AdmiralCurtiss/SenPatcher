#include "exe_patch.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>

#include "util/memwrite.h"
#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

// #define LOG_PLAY_POSITION
// #define DUMP_DECODED_BGM

#ifdef DUMP_DECODED_BGM
#include "util/file.h"
#endif

#ifdef LOG_PLAY_POSITION
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace SenLib::TX {
#ifdef LOG_PLAY_POSITION
static void __fastcall LogPlayPosition(char* prOggFile, uint32_t enginePlayPosition) {
    char buffer[512];

    sprintf(buffer,
            "Playing: LoopStart: %u, LoopEnd: %u, 0x40: %u, 0x270: %u, 0x320: %u, "
            "enginePlayPosition/4: %u \n",
            *std::bit_cast<uint32_t*>(prOggFile + 0x38),
            *std::bit_cast<uint32_t*>(prOggFile + 0x3c),
            *std::bit_cast<uint32_t*>(prOggFile + 0x40),
            *std::bit_cast<uint32_t*>(prOggFile + 0x270),
            *std::bit_cast<uint32_t*>(prOggFile + 0x320),
            enginePlayPosition / 4);

    OutputDebugStringA(buffer);
}
#endif

#ifdef DUMP_DECODED_BGM
static std::unique_ptr<HyoutaUtils::IO::File> s_DumpFile;

static void __fastcall DumpBgm(const char* buffer, uint32_t length) {
    if (!s_DumpFile) {
        s_DumpFile = std::make_unique<HyoutaUtils::IO::File>();
        s_DumpFile->Open("bgmdmp.bin", HyoutaUtils::IO::OpenMode::Write);
    }
    if (s_DumpFile->IsOpen()) {
        s_DumpFile->Write(buffer, length);
    }
}
#endif

void PatchBgmResume(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;
    using namespace SenPatcher::x86;
    using namespace HyoutaUtils::MemWrite;

    char* const streamBgmFromPrFileCallSite =
        GetCodeAddressSteamGog(version, textRegion, 0x4486cc, 0x44703c);
    char* const streamBgmFromPrFileAfterCall =
        GetCodeAddressSteamGog(version, textRegion, 0x4486d4, 0x447044);
    char* const loadOggFileCallSite =
        GetCodeAddressSteamGog(version, textRegion, 0x449027, 0x447977);
    char* const readFromPrFileOgg = GetCodeAddressSteamGog(version, textRegion, 0x409733, 0x409273);
    char* const readFromOggSkipSamplesBugStackInit =
        GetCodeAddressSteamGog(version, textRegion, 0x4097e2, 0x409322);
    char* const readFromOggSkipSamplesBugRemember =
        GetCodeAddressSteamGog(version, textRegion, 0x40981e, 0x40935e);
    char* const readFromOggSkipSamplesBugSkipForward =
        GetCodeAddressSteamGog(version, textRegion, 0x409874, 0x4093b4);
    char* const readFromOggSkipSamplesBugAdvancePosition =
        GetCodeAddressSteamGog(version, textRegion, 0x4098f1, 0x409431);

    // at the call sites, we need to pass along the parameter for the engine-tracked play position
    // in edx; this makes the other audio format overloads just ignore it.
    // note that the parameter passed here is in *bytes* while the position value in the ogg file
    // struct is in *samples*. they're uncompressed 16-bit integer samples and one per channel, so
    // multiply by 2 and then by channel count to match them.
    {
        auto injectResult = InjectJumpIntoCode<5>(logger, streamBgmFromPrFileCallSite, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();
        Emit_MOV_R32_DwordPtrR32PlusOffset(codespace, R32::EDX, R32::EBP, 0xc);
        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }
    {
        auto injectResult =
            InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, loadOggFileCallSite, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();
        // when loading ogg in one shot instead of streaming, the position is always 0
        Emit_XOR_R32_R32(codespace, R32::EDX, R32::EDX);
        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }

    {
        // in the function itself, add some initial checks to re-sync any possibly desynced play
        // position in the ogg file with the actual play position from the engine
        auto injectResult =
            InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, readFromPrFileOgg, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        overwrittenInstructions[5] += 0x20; // reserve extra stack space for temporaries
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

#ifdef LOG_PLAY_POSITION
        BranchHelper4Byte log_func;
        void* logFunc = LogPlayPosition;
        log_func.SetTarget(static_cast<char*>(logFunc));

        Emit_PUSH_R32(codespace, R32::ECX);
        Emit_PUSH_R32(codespace, R32::EDX);
        log_func.WriteJump(codespace, JumpCondition::CALL);
        Emit_POP_R32(codespace, R32::EDX);
        Emit_POP_R32(codespace, R32::ECX);
#endif

        BranchHelper1Byte go_back;

        // if we have 0 channels this can't produce anything useful so just bail
        WriteInstruction56(codespace, 0x83b90c02000000); // cmp dword ptr[ecx+20ch],0
        go_back.WriteJump(codespace, JumpCondition::JZ);

        Emit_PUSH_R32(codespace, R32::EDX);
        WriteInstruction48(codespace, 0x8b8120030000); // mov eax,dword ptr[ecx+320h] ; sample pos
        WriteInstruction16(codespace, 0xd1e0);         // shl eax,1 ; samples -> bytes
        WriteInstruction48(codespace, 0xf7a10c020000); // mul dword ptr[ecx+20ch] ; channel count
        Emit_POP_R32(codespace, R32::EDX);

        // did the play position from the engine and the play position from the ogg diverge?
        Emit_CMP_R32_R32(codespace, R32::EAX, R32::EDX);
        go_back.WriteJump(codespace, JumpCondition::JE);

        // yes, it did. reset the ogg and skip to the correct position.
        Emit_PUSH_R32(codespace, R32::ECX);
        Emit_PUSH_R32(codespace, R32::EDX);
        WriteInstruction16(codespace, 0x8b01);   // mov eax,dword ptr[ecx]
        WriteInstruction24(codespace, 0xff500c); // call dword ptr[eax+0xc] ; reset ogg
        Emit_POP_R32(codespace, R32::EAX);
        Emit_POP_R32(codespace, R32::ECX);

        // eax now contains number of bytes to skip, but we want number of samples
        Emit_XOR_R32_R32(codespace, R32::EDX, R32::EDX);
        WriteInstruction48(codespace, 0xf7b10c020000); // div dword ptr[ecx+20ch] ; channel count
        WriteInstruction16(codespace, 0xd1e8);         // shr eax,1 ; bytes -> samples

        // store samples to skip, the function will obey this and read and discard samples until
        // we're at the correct position
        WriteInstruction24(codespace, 0x894140); // mov dword ptr[ecx+40h],eax

        // everything cleaned up, go back
        go_back.SetTarget(codespace);
        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }

    // there's a logic bug in the function when skipping forward that causes it to only skip
    // forward in blocks of samples rather than exact samples. fix that.
    {
        // init stack variable that will hold the local samples to skip to zero
        auto injectResult = InjectJumpIntoCode<6, PaddingInstruction::Nop>(
            logger, readFromOggSkipSamplesBugStackInit, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();
        WriteInstruction32(codespace, 0xc7442430); // mov dword ptr[esp+0x30],0
        WriteAdvUInt32(codespace, 0);
        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }
    {
        auto injectResult = InjectJumpIntoCode<7, PaddingInstruction::Nop>(
            logger, readFromOggSkipSamplesBugRemember, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);

        // overwritten code is buggy, don't place back

        // this branch is only reached if numberOfSamplesRead >= samplesToSkip
        // ecx -> number of samples we should skip
        // ebx -> number of samples that have been read from ogg

        // store the remaining samples to skip for later
        // (so we know how much to skip forward in the source buffer)
        WriteInstruction32(codespace, 0x894c2430); // mov dword ptr[esp+0x30],ecx

        // reduce the amount of samples that we end up copying to the target buffer
        Emit_SUB_R32_R32(codespace, R32::EBX, R32::ECX);

        // clear the samples to skip so that the next iteration doesn't skip again
        Emit_XOR_R32_R32(codespace, R32::ECX, R32::ECX);

        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }
    {
        // skip forward in the read buffer by the amount of samples that we marked for skipping
        // before
        auto injectResult =
            InjectJumpIntoCode<5>(logger, readFromOggSkipSamplesBugSkipForward, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        WriteInstruction32(codespace, 0x8b4c2430); // mov ecx,dword ptr[esp+30h]
        WriteInstruction24(codespace, 0xc1e102);   // shl ecx,2
        Emit_ADD_R32_R32(codespace, R32::EDX, R32::ECX);
        Emit_MOV_R32_R32(codespace, R32::ECX, R32::EBX);

        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }
    {
        // finally, when advancing the play position and buffer, we need to add the skipped samples
        // as well
        auto injectResult = InjectJumpIntoCode<6, PaddingInstruction::Nop>(
            logger, readFromOggSkipSamplesBugAdvancePosition, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        WriteInstruction32(codespace, 0x037c2430); // add edi,dword ptr[esp+30h]

        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();
        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }

#ifdef DUMP_DECODED_BGM
    {
        auto injectResult = InjectJumpIntoCode<5>(logger, streamBgmFromPrFileAfterCall, codespace);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        BranchHelper4Byte dump_func;
        void* dumpBgmFunc = DumpBgm;
        dump_func.SetTarget(static_cast<char*>(dumpBgmFunc));

        Emit_PUSH_R32(codespace, R32::EAX);
        Emit_PUSH_R32(codespace, R32::ECX);
        Emit_PUSH_R32(codespace, R32::EDX);
        Emit_MOV_R32_R32(codespace, R32::EDX, R32::EAX);
        Emit_MOV_R32_DwordPtrR32PlusOffset(codespace, R32::ECX, R32::EBP, 0x8);
        dump_func.WriteJump(codespace, JumpCondition::CALL);
        Emit_POP_R32(codespace, R32::EDX);
        Emit_POP_R32(codespace, R32::ECX);
        Emit_POP_R32(codespace, R32::EAX);

        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();
    }
#endif

    execData.Codespace = codespace;
}
} // namespace SenLib::TX
