#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t2070[] =
    "Fix text issues in Parm (Manager's Residence)";
}

namespace SenLib::Sen3::FileFixes::t2070_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t2070.dat",
            16065,
            HyoutaUtils::Hash::SHA1FromHexString("4f6d2b47ccfd0cd0d7946459a991e7f7660cfb4e"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "When mixed correctly, it makes dye with a\x01nice, golden hue. You should be abe to
        // find\x01a big deposit along Old Agria Road."
        // missing 'l' in 'able'
        // Chapter 1 field trip first day, when starting the Prepare to Dye quest
        patcher.ExtendPartialCommand(0x25f3, 0xf2, 0x26b6, {{'l'}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t2070_dat
