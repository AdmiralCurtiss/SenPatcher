#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_pablo[] = "Fix spelling in conversation with Pablo.";
}

namespace SenLib::Sen3::FileFixes::tk_pablo_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_pablo.dat",
            16266,
            HyoutaUtils::Hash::SHA1FromHexString("ac0620f3e07e52bb5112fb13096b9d3e8f8322b9"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#kO-Orbal bikes...?! I'd heard\x01rumours, but man..."
        // rumours -> rumors
        // this is kind of arguable, since it's Pablo with his... whatever accent that is, but
        // probably better to err on the side of AE spelling
        // (chapter 2, 5/20, after geofront, field ex camp)
        patcher.RemovePartialCommand(0x36f6, 0xef, 0x3720, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_pablo_dat
