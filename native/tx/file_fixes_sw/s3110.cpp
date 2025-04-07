#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s3110[] = "Text fixes in Acros Theater.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::s3110 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s3110.dat",
            160041,
            HyoutaUtils::Hash::SHA1FromHexString("24084785156aa4b88c8702fe90dc8f8fb9fc6159"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // match memories to game:
        // "#1PAll right! Let's jump right in with\x01our first song!"
        patcher.ReplacePartialCommand(0x1ecc8,
                                      0x41,
                                      0x1ecd0,
                                      0x36,
                                      STR_SPAN("#1PLet's kick things off with our first\x01"
                                               "song of the show!"));

        // "We'll leave you guys with a brand\x01new song as our last! We worked\x01hard on it, so I
        // hope you enjoy!"
        patcher.ReplacePartialCommand(0x2206d,
                                      0x6d,
                                      0x22075,
                                      0x62,
                                      STR_SPAN("We're going to wrap things up with\x01"
                                               "a brand new song for you guys!\x01"
                                               "I hope you love it!"));

        // "#2PYeah, ever since we were trainees\x01So about five years now."
        // missing period
        patcher.ExtendPartialCommand(0xec6c, 0x9a, 0xEC98, {{'.'}});


        // "#800WAll right, let's do this...\x01#1000W#5SSPiKA!"
        // harmless
        // patcher.ReplacePartialCommand(0x9e47, 0x3a, 0x9e76, 0x9, "");

        // "#800WIt's called...\x01#1000W＂Wish★Wing＂!"
        // harmless
        // patcher.ReplacePartialCommand(0x17fc0, 0x37, 0x17fdc, 0x18, "");

        // "#800WIt's called...\x01#1000W＂Wish★Wing＂!"
        // harmless
        // patcher.ReplacePartialCommand(0x2210a, 0x37, 0x22126, 0x18, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s3110
