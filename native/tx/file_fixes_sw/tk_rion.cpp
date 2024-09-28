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

namespace SenLib::TX::FileFixesSw::tk_rion {
std::string_view GetDescription() {
    return "Text fixes in conversations with Rion.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_rion.dat",
            12017,
            HyoutaUtils::Hash::SHA1FromHexString("40fc3af8b984e1f92286bf1adcf302a01434e943"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "i did"
        // This looks like someone accidentally deleting part of a line during editing and not
        // noticing. For lack of better options, revert this to the old script for now.
        patcher.ReplacePartialCommand(0x17e1, 0xc, 0x17e6, 0x5, STR_SPAN("Really? Hehe, thanks."));


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_rion
