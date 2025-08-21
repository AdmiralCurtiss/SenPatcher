#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1230[] =
    "Fix typo in Demeter Exchange & Antiques (Saint-Arkh).";
}

namespace SenLib::Sen3::FileFixes::t1230_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t1230.dat",
            2833,
            HyoutaUtils::Hash::SHA1FromHexString("ad8a258cd3284491149efcd8e193c76eef86e66a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "There are some truly despicable\x01npeople out there..."
        // npeople -> people
        // (chapter 1, 4/22, after you come back from the forest)
        patcher.RemovePartialCommand(0x82b, 0x8e, 0x8a3, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t1230_dat
