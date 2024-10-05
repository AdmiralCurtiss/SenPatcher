#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::m9800 {
std::string_view GetDescription() {
    return "Text fixes in Boundary of Blood.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m9800.dat",
            34353,
            HyoutaUtils::Hash::SHA1FromHexString("2f18f0b48ce643d3de81b00faf4f4f895c501134"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // unicode instead of ascii '&' in 'Kou & Ryouta' nametag
        patcher.ReplacePartialCommand(0x518c, 0x12, 0x5191, 0x3, {{0x26}});

        // "#1PBeen a while since we've been to\x01the Otherworld. I'll be counting on\x01you,
        // Raging Gear!"
        // Japanese version of this line has a delay before the second sentence.
        patcher.ReplacePartialCommand(0x2e6e, 0x63, 0x2ea9, 0x1, STR_SPAN("#11W #1000W"));

        // "#1C#2P#1CIn the name of our Holy Father...\x01#11W#1000W#5SCome forth!"
        // broken text delay
        patcher.ReplacePartialCommand(0x30c5, 0x4e, 0x30f6, 0xc, STR_SPAN("#11W. #1000W\x01"));

        // "#0T#3S#300WOoo#4Sooo...\x01#32W#1000W#5SOW!"
        // broken text delay
        bin[0x387b] = 0x31;
        bin[0x388d] = 0x34;
        bin[0x388e] = 0x35;
        patcher.ShiftData(0x388c, 0x388a, 4);
        // patcher.ReplacePartialCommand(0x386c, 0x32, 0x3896, 0x6, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9800
