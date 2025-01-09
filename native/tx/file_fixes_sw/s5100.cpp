#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s5100[] = "Text fixes in abandoned factory.";
}

namespace SenLib::TX::FileFixesSw::s5100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s5100.dat",
            102817,
            HyoutaUtils::Hash::SHA1FromHexString("ed5d1837bac433855683dfd17a705dc09c2d0292"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "White shroud" -> "White Shroud"
        bin[0x68bb] = 0x53;

        // "Obtained 1,00 gems" -> "Obtained 1,000 gems"
        // to match what you actually get
        patcher.ReplacePartialCommand(0x142be, 0x23, 0x142ce, 0x5, {{0x31, 0x2c, 0x30}});

        // "#4S#800WTheir leader#50W, #800Whis lieutenants,#50W\x01#800Wthe gutter
        // trash#36W...\x01#6S#1000WWe'll kill every last one of them!"
        // patcher.ReplacePartialCommand(0x1280, 0x86, 0x12dc, 0x28, "");

        // "#4S#800WDon't tell me that you're\x01#1000W#5Slookin' to get another?!"
        // patcher.ReplacePartialCommand(0x1f57, 0x4d, 0x1f87, 0x1b, "");

        // "#600W#4SThat's it!#15W\x01#1000W#6SIt's time for a bloodbath!"
        // patcher.ReplacePartialCommand(0x2a2d, 0x44, 0x2a52, 0x1d, "");

        // "#E[9]#M_A#800WBut I guess there's no way\x01to sing without heart..."
        // patcher.ReplacePartialCommand(0x9f1f, 0xc5, 0x9faa, 0x38, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s5100
