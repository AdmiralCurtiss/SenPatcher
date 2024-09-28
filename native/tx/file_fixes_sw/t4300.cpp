#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::t4300 {
std::string_view GetDescription() {
    return "Text fixes in Le Coultre Antique Shop.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t4300.dat",
            104433,
            HyoutaUtils::Hash::SHA1FromHexString("514c40c1df4fc8f7055dd657b838a770971b9fc6"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "So a greed different from the phantom\x01hands has appeared in the fog"
        // missing period at the end
        patcher.ExtendPartialCommand(0x108a5, 0x96, 0x108fa, {{0x2e}});

        // fix the "About Attachments" tip not losing it's 'new' icon after reading it
        patcher.RemovePartialCommand(0x572a, 0x1c, 0x573e, 0x5);

        // TODO: there's also a ton of text color issues in the tips...

        // "Fair. You ready to start now?"
        // this is the first line of a conversation, so remove the "Fair."
        patcher.RemovePartialCommand(0x114e8, 0x64, 0x114ed, 0x6);

        // "Don't get cocky, but, with your strength, I trust you can take it down"
        // missing period
        patcher.ExtendPartialCommand(0x3f01, 0x77, 0x3f4c, {{0x2e}});


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t4300
