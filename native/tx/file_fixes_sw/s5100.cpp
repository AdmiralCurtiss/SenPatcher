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
            102833,
            HyoutaUtils::Hash::SHA1FromHexString("8a1b8f5a23c1e917d918eb11af09a6663932f31d"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "White shroud" -> "White Shroud"
        // (EV_06_09_02)
        bin[0x68bb] = 0x53;

        // "Obtained 1,00 gems" -> "Obtained 1,000 gems"
        // to match what you actually get
        // we overwrite a duplicate color tag so it fits in-place
        // (during the student movie quest in the epilogue)
        patcher.ReplacePartialCommand(0x142cb, 0x23, 0x142db, 0x5, {{'1', ',', '0'}});

        // "##2P...Wouldn't it be a nice surprise if this was\x01
        // actually a movie shoot or something?"
        // double '#' in front
        // (during the Spika-Boo quest in the final chapter)
        patcher.RemovePartialCommand(0xe765, 0x60, 0xe76e, 1);

        // "#4S#800WTheir leader#50W, #800Whis lieutenants,#50W#800Wthe\x01gutter
        // trash...#36W\x01#6S#1000WWe'll kill every last one of them!"
        // Missing space and a lot of dubious text speed tags. This actually isn't too bad in game
        // but since we're already fixing the space anyway...
        // (EV_03_18_02)
        {
            std::vector<char> tmp;
            tmp.assign(&bin[0x1280], &bin[0x1280] + 0x85);
            tmp[0x43] = ' ';
            tmp.erase(tmp.begin() + 0x53, tmp.begin() + 0x53 + 4);
            tmp.erase(tmp.begin() + 0x37, tmp.begin() + 0x37 + 9);
            tmp.insert(tmp.begin() + 0x37, ' ');
            tmp.erase(tmp.begin() + 0x22, tmp.begin() + 0x22 + 5);
            tmp.erase(tmp.begin() + 0x1c, tmp.begin() + 0x1c + 4);
            patcher.ReplaceCommand(0x1280, 0x85, tmp);
        }

        // "#4S#800WDon't tell me that you're\x01#1000W#5Slookin' to get another?!"
        // harmless
        // (EV_03_18_02)
        // patcher.ReplacePartialCommand(0x1f56, 0x4d, 0x1f86, 0x1b, "");

        // "#600W#4SThat's it!#15W\x01#1000W#6SIt's time for a bloodbath!"
        // harmless
        // (EV_03_18_02)
        // patcher.ReplacePartialCommand(0x2a2c, 0x44, 0x2a51, 0x1d, "");

        // "#E[9]#M_A#800WBut I guess there's no way\x01to sing without heart..."
        // harmless
        // (EV_06_09_02)
        // patcher.ReplacePartialCommand(0x9f20, 0xc5, 0x9fab, 0x38, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s5100
