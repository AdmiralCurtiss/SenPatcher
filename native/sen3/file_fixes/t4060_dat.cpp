#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t4060[] = "Fix extra word in Raquel (casino)";
}

namespace SenLib::Sen3::FileFixes::t4060_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t4060.dat",
            87137,
            HyoutaUtils::Hash::SHA1FromHexString("f37dfc180fcd77ef6a7283c125c2ac63e0b58613"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "Well, this is in an interesting turn of events."
        // is in an -> is an
        // (chapter 3, 6/17 night, cutscene with Zephyr playing Vantage Masters)
        patcher.RemovePartialCommand(0xe277, 0x38, 0xe28c, 3);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t4060_dat
