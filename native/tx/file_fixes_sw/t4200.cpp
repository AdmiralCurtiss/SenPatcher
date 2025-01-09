#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t4200[] = "Text fixes in Caf\xc3\xa9 17.";
}

namespace SenLib::TX::FileFixesSw::t4200 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t4200.dat",
            86673,
            HyoutaUtils::Hash::SHA1FromHexString("2275260b6f44505d8365702986c51c477f6dc214"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E[333333333333332]#M_0#1PAhem!\x01#1000W#5SHappy birthday, Asuka!"
        // patcher.ReplacePartialCommand(0x1014c, 0x49, 0x1017a, 0x19, "");

        // "#1PMay 18th. #1000WThat's your 17th\x01birthday, yeah?"
        // patcher.ReplacePartialCommand(0x104ea, 0x3d, 0x104ff, 0x26, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t4200
