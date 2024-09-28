#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::t2000 {
std::string_view GetDescription() {
    return "Text fixes in Station Square.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t2000.dat",
            326081,
            HyoutaUtils::Hash::SHA1FromHexString("7320d0dbd78bb281687fc525e88dd23b346341f2"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#4K#FWh-What...the-#1000W\x01#5SWh-WHOOOOOOAAAAAA!"
        // reintroduce missing text delay
        patcher.ExtendPartialCommand(0x2d6e5, 0x37, 0x2d6fa, {{0x23, 0x33, 0x30, 0x57}});

        // "#2PWait! Take a second and...#45Wthink...\x01#1000W#5SGod damn it, fine!"
        // text print speed error, as usual JP only adjusts this for a single char to introduce a
        // short delay between the two sentence fragments
        bin[0x2db60] = 0x30;
        bin[0x2db61] = 0x33;
        patcher.ShiftData(0x2db5f, 0x2db6a, 4);
        patcher.ShiftData(0x2db6c, 0x2db6b, 6);

        // "He passed away two years go." -> ago
        patcher.ExtendPartialCommand(0x3b114, 0x8d, 0x3b13a, {{0x61}});

        // "I feel like a lot of unnatural phenomena has been happening the past few days."
        // -> have been
        patcher.ReplacePartialCommand(0xd460, 0x9f, 0xd48e, 0x1, {{0x76, 0x65}});


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t2000
