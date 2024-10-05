#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t1310 {
std::string_view GetDescription() {
    return "Text fixes in school clubhouse.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1310.dat",
            207185,
            HyoutaUtils::Hash::SHA1FromHexString("e1a59579e1637ef2b78dbccc81acb08393398233"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // linebreaks
        std::swap(bin[0xea19], bin[0xea21]);
        std::swap(bin[0x2c549], bin[0x2c54e]);
        std::swap(bin[0x2c91d], bin[0x2c923]);

        // For completion, see t1210...
        // "Huh. Heard you guys were in the area, but I didn't expect you to be HERE."
        // "What're you guys doing in the dojo, of all places?"
        // These are written as if they were part of the same conversation, but they're actually
        // mutually exclusive lines depending on if you've heard about Spika being at school before
        // or not. The first one is 'have heard' and the second one is 'have not heard'.
        // These are pretty good as-is, but the follow-up line from Wakaba has 'it' referring to the
        // dojo, so it needs to be mentioned explicitly in the first variant.
        std::swap(bin[0x2c042], bin[0x2c046]);
        patcher.ReplacePartialCommand(0x2c017, 0x51, 0x2c061, 4, STR_SPAN("in the dojo"));
        std::swap(bin[0x2c09a], bin[0x2c0a7]);
        patcher.ExtendPartialCommand(0x2c07e, 0x3a, 0x2c09a, STR_SPAN(" HERE"));



        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1310
