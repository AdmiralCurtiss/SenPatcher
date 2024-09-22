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

namespace SenLib::TX::FileFixesSw::t3100 {
std::string_view GetDescription() {
    return "Text fixes in Kuguraya.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t3100.dat",
            110721,
            HyoutaUtils::Hash::SHA1FromHexString("63fcba615c3111ce302665f97595eeb0841c3bfe"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "...#14WWhat is going on\x01between you and Kou, Asuka?"
        // that #14W is a command to reduce the speed at which the text is printed, it's a leftover
        // from the JP script which only sets it for one character and then back to the standard
        // speed in order to add a delay mid-dialogue to, probably, match the voice acting.
        // remove it. also move the linebreak.
        std::swap(bin[0xeda1], bin[0xeda9]);
        patcher.RemovePartialCommand(0xed82, 0x3d, 0xed8d, 0x4);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t3100
