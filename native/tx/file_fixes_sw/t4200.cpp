#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t4200[] = "Text fixes in Caf\xc3\xa9 17.";
}

namespace SenLib::TX::FileFixesSw::t4200 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t4200.dat",
            86689,
            HyoutaUtils::Hash::SHA1FromHexString("c804f1d8de11aedc25ba8fd560cf06f9c2a13780"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E[5]#M_0And it looks like you have,too. Feel free\x01to sit down and relax if you want
        // to."
        // missing space after comma
        // (NPC dialogue with Yamaoka in After Story on 10/30)
        patcher.ExtendPartialCommand(0x4a86, 0x12d, 0x4B7D, {{' '}});

        // "Seriously. No one even knows diddly squat-about\x01what really happened with those gas
        // pipes."
        // the dash got misplaced here
        // (NPC dialogue with Chiemi in Chapter 6 on 6/26)
        // patcher.ReplacePartialCommand(0x797b, 0x19b, 0x797e, 0x134, "");
        std::swap(bin[0x7A7B], bin[0x7A81]);

        // "#E[333333333333332]#M_0#1PAhem!\x01#1000W#5SHappy birthday, Asuka!"
        // harmless
        // (EV_12_04_01)
        // patcher.ReplacePartialCommand(0x10160, 0x49, 0x1018e, 0x19, "");

        // "#1PMay 18th. #1000WThat's your 17th\x01birthday, yeah?"
        // harmless
        // (EV_12_04_01)
        // patcher.ReplacePartialCommand(0x104fe, 0x3d, 0x10513, 0x26, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t4200
