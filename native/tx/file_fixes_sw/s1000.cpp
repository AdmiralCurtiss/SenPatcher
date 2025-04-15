#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s1000[] = "Text fixes in Kokonoe Shrine (outside).";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::s1000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s1000.dat",
            137113,
            HyoutaUtils::Hash::SHA1FromHexString("b9c1b1a7d7b6a860f08316058b11c24231825783"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1P...!#10W\x01#1000W#5SWhat is THAT?!"
        // harmless
        // (EV_08_00_01)
        // patcher.ReplacePartialCommand(0x13609, 0x2d, 0x13623, 0x11, "");

        // "I'll protect, Grandma!"
        // add a 'you'
        // (before entering Pandora, small kid in the corner)
        patcher.ExtendPartialCommand(0xdc79, 0x63, 0xDCD0, STR_SPAN(" you"));

        // "(＂I hope I can score a date with the\x01girl standing next to me right now…＂)"
        // ellipsis -> three dots
        // patcher.ReplacePartialCommand(0xbeb6, 0x55, 0xbeb9, 0x50, "");
        bin[0xBF02] = '.';
        bin[0xBF03] = '.';
        bin[0xBF04] = '.';

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s1000
