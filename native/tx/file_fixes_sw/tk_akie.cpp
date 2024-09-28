#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::tk_akie {
std::string_view GetDescription() {
    return "Text fixes in conversations with Akie.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_akie.dat",
            10306,
            HyoutaUtils::Hash::SHA1FromHexString("0add3f39da328f134022969871f6e8111906c5ca"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "I heard they're having an anniversary concert today."
        // Wrong timeframe. It wasn't 'today' when I got this text, and the JP is not specific about
        // the time, so let's just change that to 'soon'.
        patcher.ReplacePartialCommand(0x122d, 0x134, 0x133a, 0x5, {{0x73, 0x6f, 0x6f, 0x6e}});
        patcher.ReplacePartialCommand(0x137a, 0x94, 0x13e2, 0x5, {{0x73, 0x6f, 0x6f, 0x6e}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_akie
