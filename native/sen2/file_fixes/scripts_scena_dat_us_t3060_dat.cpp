#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3060[] =
    "Fix text errors in Viscount Arseid's Mansion.";
}

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t3060_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t3060.dat",
            39585,
            HyoutaUtils::Hash::SHA1FromHexString("7da332f0c421b6796b4c2e0cc53b570c4a4d974e"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // remove duplicate 'how'
        patcher.RemovePartialCommand(0x3f00, 0x119, 0x3faa, 0x4);

        // add voice clip
        // this is in act 1 part 3, the scene with Aurelia and Wallace
        patcher.ExtendPartialCommand(0x67c0, 0xd, 0x67c8, {{0x11, 0x4e, (char)0xff, 0x00, 0x00}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t3060_dat
