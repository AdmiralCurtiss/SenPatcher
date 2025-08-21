#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_agate[] =
    "Fix typography errors in conversations with Agate.";
}

namespace SenLib::Sen3::FileFixes::tk_agate_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_agate.dat",
            810,
            HyoutaUtils::Hash::SHA1FromHexString("b41338fb186a459b21f6b3e89de4eadb0ffc3352"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E_0#M_A#B_0I’ll be going on patrol after\x01this. You guys be careful."
        // wrong apostrophe
        // (final chapter, at the heimdallr bracer guild)
        patcher.ReplacePartialCommand(0x6b, 0x11a, 0x14a, 3, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_agate_dat
