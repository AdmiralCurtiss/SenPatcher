#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t4000[] = "Text fixes in Brick Alley.";
}

namespace SenLib::TX::FileFixesSw::t4000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t4000.dat",
            321377,
            HyoutaUtils::Hash::SHA1FromHexString("7805471fda9c253233472ff4e38c8e5fb8c751b6"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "But they would'v made the atmosphere\x01ominous and oppressive, so we\x01canned the
        // idea."
        // -> "would've"
        // (Nemoto in Chapter 6 on 6/27, after talking to Spika at the Acros Theater about Rion's
        // illness)
        patcher.ExtendPartialCommand(0xcb82, 0xb0, 0xCBED, {{'e'}});

        // "#E[3]#M_A#3K#800WWhat are you even doing here#12W,\x01#200WKugayama?"
        // harmless. pretty good, actually.
        // (EV_03_04_01)
        // patcher.ReplacePartialCommand(0x1b9c7, 0x4b, 0x1ba02, 0xe, "");

        // "#E[3]#M_0#2PI may have no clue WHY we're even here\x01in the first place…"
        // ellipsis -> three dots
        // (EV_17_20_03)
        // patcher.ReplacePartialCommand(0x39412, 0x4d, 0x3941e, 0x3f, "");
        bin[0x3945A] = '.';
        bin[0x3945B] = '.';
        bin[0x3945C] = '.';


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t4000
