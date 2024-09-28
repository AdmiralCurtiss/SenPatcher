#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::s5100 {
std::string_view GetDescription() {
    return "Text fixes in abandoned factory.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s5100.dat",
            102817,
            HyoutaUtils::Hash::SHA1FromHexString("ed5d1837bac433855683dfd17a705dc09c2d0292"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "White shroud" -> "White Shroud"
        bin[0x68bb] = 0x53;

        // "Obtained 1,00 gems" -> "Obtained 1,000 gems"
        // to match what you actually get
        patcher.ReplacePartialCommand(0x142be, 0x23, 0x142ce, 0x5, {{0x31, 0x2c, 0x30}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s5100
