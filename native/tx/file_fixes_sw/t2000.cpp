#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t2000[] = "Text fixes in Station Square.";
}

namespace SenLib::TX::FileFixesSw::t2000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t2000.dat",
            326081,
            HyoutaUtils::Hash::SHA1FromHexString("1cb989a2a90d0a1589bc9bf31298d661231f8cbf"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#4K#FWh-What...the─#1000W\x01#5SWh-WHOOOOOOAAAAAA!"
        // reintroduce missing text delay
        // Chapter 4 Side Story (EV_13_02_03)
        patcher.ExtendPartialCommand(0x2d6d9, 0x37, 0x2d6ee, {{0x23, 0x33, 0x30, 0x57}});

        // "#2PWait! Take a second and...#45Wthink...\x01#1000W#5SGod damn it, fine!"
        // text print speed error, as usual JP only adjusts this for a single char to introduce a
        // short delay between the two sentence fragments
        // Chapter 4 Side Story (EV_13_02_03)
        bin[0x2db60 - 0xc] = 0x30;
        bin[0x2db61 - 0xc] = 0x33;
        patcher.ShiftData(0x2db5f - 0xc, 0x2db6a - 0xc, 4);
        patcher.ShiftData(0x2db6c - 0xc, 0x2db6b - 0xc, 6);

        // "#K#0TYes. He passed away two years go."
        // -> ago
        // during the 'Treasure That Transcends Time' quest (QS_0703_01)
        patcher.ExtendPartialCommand(0x3b10c, 0x8d, 0x3b132, {{0x61}});

        // "I feel like a lot of unnatural\x01phenomena has been happening\x01the past few days."
        // -> have been
        // NPC dialogue with Sergeant Takahashi on 7/8 (talk to him twice)
        patcher.ReplacePartialCommand(0xd464, 0x9f, 0xd492, 0x1, {{0x76, 0x65}});

        // "#E[3333333333333336]#M_A#1PMy guess is...#1000Weveryone\x01in the city disappeared."
        // harmless
        // patcher.ReplacePartialCommand(0x304b0, 0x5a, 0x304e1, 0x27, "");

        // "#800W#4STokyo and its surrounding cities,\x01 Morimiya included, will
        // experience\x01intermittent days of heavy fog."
        // misplaced space
        // (EV_04_00_00)
        patcher.RemovePartialCommand(0x20f2a, 0x75, 0x20F58, 1);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t2000
