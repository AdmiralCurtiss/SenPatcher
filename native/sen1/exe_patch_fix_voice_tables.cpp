#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
static const char FormattingStringJp[] = "%s/text/dat/%s.tbl";
static const char FormattingStringEn[] = "%s/text/dat_us/%s.tbl";

void PatchLanguageAppropriateVoiceTables(SenPatcher::Logger& logger,
                                         char* textRegion,
                                         GameVersion version,
                                         char*& codespace,
                                         char* codespaceEnd) {
    using namespace SenPatcher::x86;
    char* const t_vctiming_address = GetCodeAddressJpEn(version, textRegion, 0xb41c68, 0xb43f30);
    char* const t_voice_address = GetCodeAddressJpEn(version, textRegion, 0xb41bdc, 0xb43ea4);
    char* const GetPcConfigAddress = GetCodeAddressJpEn(version, textRegion, 0x7bdef0, 0x7bf7c0);
    char* const GetVoiceLangAddress = GetCodeAddressJpEn(version, textRegion, 0x7bd3f0, 0x7becc0);
    char* const InjectAddress = GetCodeAddressJpEn(version, textRegion, 0x56f6d8, 0x5708d8);
    const char* const FormattingStringJpAddress = &FormattingStringJp[0];
    const char* const FormattingStringEnAddress = &FormattingStringEn[0];
    const bool jp = (version == GameVersion::Japanese);
    using JC = JumpCondition;

    {
        BranchHelper4Byte back_to_function;
        BranchHelper4Byte get_pc_config;
        BranchHelper4Byte pc_config__get_voice_language;
        BranchHelper1Byte depends_on_voice_lang;
        BranchHelper1Byte exit_inject;
        BranchHelper1Byte use_english_string;
        BranchHelper1Byte use_text_lang_string;

        get_pc_config.SetTarget(GetPcConfigAddress);
        pc_config__get_voice_language.SetTarget(GetVoiceLangAddress);

        // inject into the asset-from-text-subfolder loader function
        char*& tmp = codespace;
        const auto injectResult =
            InjectJumpIntoCode<5, PaddingInstruction::Nop>(logger, InjectAddress, tmp);
        back_to_function.SetTarget(injectResult.JumpBackAddress);

        // assemble logic to select the voice-language-matching voice tables
        WriteInstruction16(tmp, 0x81ff); // cmp  edi,(t_vctiming)
        std::memcpy(tmp, &t_vctiming_address, 4);
        tmp += 4;
        depends_on_voice_lang.WriteJump(tmp, JC::JE); // je   depends_on_voice_lang
        WriteInstruction16(tmp, 0x81ff);              // cmp  edi,(t_voice)
        std::memcpy(tmp, &t_voice_address, 4);
        tmp += 4;
        depends_on_voice_lang.WriteJump(tmp, JC::JE); // je   depends_on_voice_lang
        use_text_lang_string.WriteJump(tmp, JC::JMP); // jmp  use_text_lang_string

        depends_on_voice_lang.SetTarget(tmp);
        get_pc_config.WriteJump(tmp, JC::CALL); // call get_pc_config
        WriteInstruction16(tmp, 0x8bc8);        // mov  ecx,eax
        pc_config__get_voice_language.WriteJump(tmp,
                                                JC::CALL); // call pc_config__get_voice_language
        WriteInstruction16(tmp, 0x84c0);                   // test al,al
        use_english_string.WriteJump(tmp, JC::JZ);         // jz   use_english_string
        if (jp) {
            use_text_lang_string.SetTarget(tmp);
        }
        WriteInstruction8(tmp, 0x68); // push (jp_formatting_string)
        std::memcpy(tmp, &FormattingStringJpAddress, 4);
        tmp += 4;
        exit_inject.WriteJump(tmp, JC::JMP); // jmp  exit_inject

        use_english_string.SetTarget(tmp);
        if (!jp) {
            use_text_lang_string.SetTarget(tmp);
        }
        WriteInstruction8(tmp, 0x68); // push (en_formatting_string)
        std::memcpy(tmp, &FormattingStringEnAddress, 4);
        tmp += 4;

        exit_inject.SetTarget(tmp);
        back_to_function.WriteJump(tmp, JC::JMP); // jmp  back_to_function
    }
}
} // namespace SenLib::Sen1
