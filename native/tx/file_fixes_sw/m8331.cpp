#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

#include "dungeon_names.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m8331[] =
    "Text fixes in " DUNGEON_NAME_m8330 " (miniboss room).";
}

namespace SenLib::TX::FileFixesSw::m8331 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m8331.dat",
            13657,
            HyoutaUtils::Hash::SHA1FromHexString("7dfff0ef6f4e701621cea565e77671f04915f7bd"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E[1]#M_4The other should be wrapping things\x01up with their own paths around now."
        // add an 's' to 'other'
        // (EV_08_24_00, Yuuki must be in party)
        patcher.ExtendPartialCommand(0x1330, 0x77, 0x1367, {{'s'}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m8331
