#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t6400[] = "Text fixes in Gemini.";
}

namespace SenLib::TX::FileFixesSw::t6400 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t6400.dat",
            76641,
            HyoutaUtils::Hash::SHA1FromHexString("7a9b023caaff5d47924e46736bd92948f2d1a774"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#4S#800WGood, then bring it on,#5W\x01#1000W#6SShio!"
        // harmless
        // (EV_03_01_00)
        // patcher.ReplacePartialCommand(0x4694, 0x3c, 0x46c5, 0x8, "");

        // "#2P#500WSo I'll ask you one more time:#15W\x01#1000W#5SPlease let us through."
        // harmless
        // (EV_03_11_00, Sora in party)
        // patcher.ReplacePartialCommand(0x6e6a, 0x55, 0x6ea3, 0x19, "");

        // "#2P#500WWhat?#14W\x01#1000W#5SYou got a problem with that?"
        // harmless
        // (EV_03_11_00)
        // patcher.ReplacePartialCommand(0x889a, 0x42, 0x88ba, 0x1f, "");

        // "#2K#F#0T...Takahane's underboss?!#10W\x01#1000W(No wonder he was so terrifying...)"
        // harmless
        // (EV_03_15_00)
        // patcher.ReplacePartialCommand(0xabd6, 0x59, 0xac04, 0x29, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t6400
