#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m9800[] = "Text fixes in Boundary of Blood.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::m9800 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m9800.dat",
            34337,
            HyoutaUtils::Hash::SHA1FromHexString("82be7798097db5100a6798239fa580587aaa8b75"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // unicode '＆' instead of ascii '&' in 'Kou ＆ Ryouta' nametag
        // (EV_17_06_01)
        patcher.ReplacePartialCommand(0x5187, 0x12, 0x518c, 0x3, {{'&'}});

        // "#1PBeen a while since we've been to\x01the Otherworld. I'll be counting on\x01you,
        // Raging Gear!"
        // Japanese version of this line has a delay before the second sentence.
        // We'll ignore this...
        // (EV_17_06_01)
        // patcher.ReplacePartialCommand(0x2e6e, 0x63, 0x2ea9, 0x1, STR_SPAN("#11W #1000W"));

        // "#1C#2P#1CIn the name of our Holy Father...\x01#11W#1000W#5SCome forth!"
        // harmless
        // (EV_17_06_01)
        // patcher.ReplacePartialCommand(0x30c5, 0x4e, 0x30f6, 0xc, STR_SPAN("#11W. #1000W\x01"));

        // "#0T#3S#300WOooooo...\x01#32W#1000W#5SOW!"
        // harmless
        // (EV_17_06_01)
        // bin[0x387b] = '1';
        // bin[0x388a] = '4';
        // bin[0x388b] = '5';
        // patcher.ShiftData(0x3889, 0x3887, 4);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9800
