#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t4300[] = "Text fixes in Le Coultre Antique Shop.";
}

namespace SenLib::TX::FileFixesSw::t4300 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t4300.dat",
            104417,
            HyoutaUtils::Hash::SHA1FromHexString("4f7c3e52104a2fbe48f843b28fa689866bd4766f"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // fix the "About Attachments" tip not losing its 'new' icon after reading it
        patcher.RemovePartialCommand(0x572d, 0x1c, 0x5741, 0x5);

        // text colors all look fixed in Switch v1.0.1, yay

        // linebreaks
        // "#E_8#M_9Gonna be meeting everyone\x01at night, but that's it, really."
        std::swap(bin[0x12726], bin[0x12729]);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t4300
