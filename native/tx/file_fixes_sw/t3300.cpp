#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3300[] = "Text fixes in Gen Soba Shop.";
}

namespace SenLib::TX::FileFixesSw::t3300 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t3300.dat",
            95425,
            HyoutaUtils::Hash::SHA1FromHexString("c72dcdfa165473c3f22fc28e92e0df13c4becc0c"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Speaking of which, from what I've heard, a pretty rough cold has been going around at
        // Morimiya Academy."
        // This is the first line in a conversation, so the 'Speaking of which' makes no sense.
        // Just remove it.
        bin[0x8144] = 0x46;
        std::swap(bin[0x8152], bin[0x8162]);
        std::swap(bin[0x8176], bin[0x8183]);
        patcher.RemovePartialCommand(0x812e, 0x124, 0x8131, 0x13);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t3300
