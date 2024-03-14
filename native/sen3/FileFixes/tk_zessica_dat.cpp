#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen3::FileFixes::tk_zessica_dat {
std::string_view GetDescription() {
    return "Fix typo in conversation with Jessica.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_zessica.dat",
            15102,
            SenPatcher::SHA1FromHexString("8a9a5ca2e438e36ea47c147124623407ae9b6b5b"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // Nevermind -> Never mind
        patcher.ExtendPartialCommand(0x1bec, 0x9e, 0x1c83, {{0x20}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_zessica_dat
