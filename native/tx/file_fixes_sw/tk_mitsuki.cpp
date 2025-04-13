#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_mitsuki[] =
    "Text fixes in conversations with Mitsuki.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::tk_mitsuki {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_mitsuki.dat",
            16153,
            HyoutaUtils::Hash::SHA1FromHexString("c80365c467026e0e9d0d25d1f35b6955ad8d364a"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#KAh... It's all right. It is a little\x01disappointing, though,I must admit."
        // missing space
        // (Intermission, after the big talk, talk to everyone, then after talking to Asuka but
        // before going into the hot springs talk to Mitsuki again)
        patcher.ExtendPartialCommand(0x1f19, 0x4f, 0x1F59, {{' '}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_mitsuki
