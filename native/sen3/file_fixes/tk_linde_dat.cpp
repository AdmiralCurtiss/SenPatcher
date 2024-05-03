#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen3::FileFixes::tk_linde_dat {
std::string_view GetDescription() {
    return "Terminology fixes in conversations with Linde.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_linde.dat",
            10685,
            HyoutaUtils::Hash::SHA1FromHexString("65aa739d1b7a5f3ff91e7fcf4840a5db26bfcdfb"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // St. Ursula's -> St. Ursula (after picking her for the radio sidequest in chapter 4)
        patcher.RemovePartialCommand(0xac3, 0xcd, 0xb54, 0x2);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_linde_dat
