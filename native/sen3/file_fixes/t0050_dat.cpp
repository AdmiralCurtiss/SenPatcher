#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0050[] = "Fix typography in Leeves (Nyo-Sui-An)";
}

namespace SenLib::Sen3::FileFixes::t0050_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0050.dat",
            40465,
            HyoutaUtils::Hash::SHA1FromHexString("a1642648e2d7a622818b101a750730a67bf8ea2d"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#4KEastern tea!? That's a great idea!"
        // !? -> ?!
        // (chapter 3, 6/11 afternoon, during the Precious Gifts for Precious People sidequest)
        // patcher.ReplacePartialCommand(0x7252, 0x6f, 0x7259, 0x25, "");
        std::swap(bin[0x7267], bin[0x7268]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0050_dat
