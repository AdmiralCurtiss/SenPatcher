#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_rosine[] = "Fix typos in conversations with Rosine.";
}

namespace SenLib::Sen2::FileFixes::scripts_talk_dat_us_tk_rosine_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_rosine.dat",
            13161,
            HyoutaUtils::Hash::SHA1FromHexString("ea737c88d0648621c297ed0c139348aa6213a40d"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // extra comma
        patcher.RemovePartialCommand(0x2506, 0x4e, 0x253f, 0x1);

        // fully fledged -> fully-fledged
        bin[0x2627] = 0x2d;

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_talk_dat_us_tk_rosine_dat
