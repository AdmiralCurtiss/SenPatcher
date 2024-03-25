#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_angelica_dat {
std::string_view GetDescription() {
    return "Fix extra word in conversation with Angelica.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_angelica.dat",
            9480,
            SenPatcher::SHA1FromHexString("e5f61f3108f8e1d43ca539b93bac91cfe37b709a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // remove extra 'to'
        // this should be right but I can't find this line in-game, so let's disable it for now...
        // patcher.RemovePartialCommand(0x619, 0x8f, 0x676, 0x3);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_angelica_dat
