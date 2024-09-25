#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::s2910 {
std::string_view GetDescription() {
    return "Text fixes in Skateboarding minigame.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s2910.dat",
            15481,
            HyoutaUtils::Hash::SHA1FromHexString("904d8f49aac9a7a14daa016117ecb54e97eb55b1"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // The quartz you get from beating Course A: Advanced is Raiden, but the message claims you
        // get Illuminati because it's checking the wrong item ID for the name. Fix that.
        bin[0x31e1] = 0x32;
        bin[0x31e2] = 0x0d;

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s2910
