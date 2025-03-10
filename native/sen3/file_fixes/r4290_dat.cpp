#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_r4290[] =
    "Fix name consistency issues in Heimdallr Ex. Camp.";
}

namespace SenLib::Sen3::FileFixes::r4290_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r4290.dat",
            98145,
            HyoutaUtils::Hash::SHA1FromHexString("871c9c02460dde4acbb7712111af384ed76a3bdc"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // General Aurelia -> General Le Guin
        using namespace HyoutaUtils::Vector;
        WriteAt(bin, 0x15ef0, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});

        // General Aurelia -> Aurelia
        patcher.RemovePartialCommand(0x13d0d, 0x53, 0x13d19, 8);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r4290_dat
