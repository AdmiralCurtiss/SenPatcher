#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_system[] =
    "Fix grammar error in message when reporting cryptids.";
}

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_system_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/system.dat",
            85857,
            HyoutaUtils::Hash::SHA1FromHexString("2a5b55ee43ae4498eccb409a491c7a6ea1e4471a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // 'been' -> 'is'
        patcher.ReplacePartialCommand(0xfcc4, 0x82, 0xfd01, 0x4, {{0x69, 0x73}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_system_dat
