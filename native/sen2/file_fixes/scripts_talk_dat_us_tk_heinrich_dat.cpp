#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_talk_dat_us_tk_heinrich_dat {
std::string_view GetDescription() {
    return "Fix missing period in conversation with Vice Principal Heinrich.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_heinrich.dat",
            7459,
            SenPatcher::SHA1FromHexString("1e88dbcff39609facaaca7a29803247739ff1f14"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // missing '.' at the end of sentence
        patcher.ExtendPartialCommand(0xed8, 0x131, 0x1007, {{0x2e}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_talk_dat_us_tk_heinrich_dat
