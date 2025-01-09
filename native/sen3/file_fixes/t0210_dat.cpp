#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0210[] =
    "Terminology fixes in Branch Campus Main Building.";
}

namespace SenLib::Sen3::FileFixes::t0210_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0210.dat",
            534945,
            HyoutaUtils::Hash::SHA1FromHexString("c45eb8312b11f6a87476165fe69d646789d5e48b"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        using namespace HyoutaUtils::Vector;

        SenScriptPatcher patcher(bin);

        // Intelligence Agency -> Intelligence Division
        patcher.ReplacePartialCommand(0x1ec86, 0x9a, 0x1ed0c, 0x6, GetSpan(bin, 0x1e848, 8));

        // synch -> sync
        patcher.RemovePartialCommand(0x4b1e8, 0x48, 0x4b213, 0x1);

        // PS4 patch script has two more changes here, which I believe are both incorrect.
        // One is the controller button icon in the tutorial message for how to use the Arcus menu,
        // which appears to already work correctly in the PC version.
        // The other one is capitalizing Port City (which they've done in lots of other places too),
        // but based on the context I think this one doesn't make any sense. It's not referring to
        // Ordis in particular, but rather port cities in general (the line is "That's the biggest
        // port city on the west coast, right?"), so I think leaving it lowercase makes more sense.

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0210_dat
