#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen3::FileFixes::tk_stark_dat {
std::string_view GetDescription() {
    return "Terminology fixes in conversations with Stark.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_stark.dat",
            29257,
            SenPatcher::SHA1FromHexString("095615a115f3653b2ecb59a16a756f8c3f12b68d"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // the Juno Naval Fortress -> Juno Naval Fortress (chapter 3 field exercises day 1, camp)
        patcher.RemovePartialCommand(0x5eb3, 0x152, 0x5f31, 0x4);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_stark_dat
