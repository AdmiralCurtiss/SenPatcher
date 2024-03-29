#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_vandyck_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in conversation with Vandyck.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_vandyck.dat",
            17811,
            SenPatcher::SHA1FromHexString("b838141d25f707a7c95191db2f8c324a3e0a34c0"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);
        patcher.RemovePartialCommand(0x1312, 0x183, 0x1420, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_vandyck_dat
