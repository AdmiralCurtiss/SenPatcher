#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t2100[] = "Text fixes in Star Camera.";
}

namespace SenLib::TX::FileFixesSw::t2100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t2100.dat",
            112785,
            HyoutaUtils::Hash::SHA1FromHexString("f8396bb757175f207430e54eb33fd4da29f37e03"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E[G]#M_0#H[0]#1P#500WYuuki... #1000WI..."
        // patcher.ReplacePartialCommand(0xe8f9, 0x34, 0xe920, 0xa, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t2100
