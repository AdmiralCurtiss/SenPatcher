#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_talk_dat_us_tk_beryl_dat {
std::string_view GetDescription() {
    return "Fix typo in conversation with Beryl.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_beryl.dat",
            11655,
            HyoutaUtils::Hash::SHA1FromHexString("73f84be48f549fb71ac265a5d7519f3d8ca16f1d"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // 'writing deep within the abyss' -> 'writhing deep within the abyss'
        patcher.ExtendPartialCommand(0xfdd, 0xac, 0xffb, {{0x68}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_talk_dat_us_tk_beryl_dat
