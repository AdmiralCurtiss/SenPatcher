#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_talk_dat_us_tk_emily_dat {
std::string_view GetDescription() {
    return "Fix typo in conversation with Emily.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_emily.dat",
            8169,
            SenPatcher::SHA1FromHexString("ea9e2d4575e334782fde96e3fa4d0c3fd5545af6"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // 'Class VIII' -> 'Class VII'
        patcher.RemovePartialCommand(0x1199, 0x6c, 0x11f7, 0x1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_talk_dat_us_tk_emily_dat
