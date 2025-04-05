#include <array>
#include <span>
#include <string_view>
#include <vector>

#include "sen/decompress_helper.h"
#include "sen/file_getter.h"
#include "tx/file_fixes.h"
#include "tx/patch_pkg.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_E_EFOBJ078[] =
    "Reinsert missing Japanese textures when starting a New Game.";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_tx_efobj078a_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::TX::FileFixes::E_EFOBJ078 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }

        {
            std::array<SingleFilePatchInfo, 1> patches{{SingleFilePatchInfo{
                .PatchData = patch->data(), .PatchLength = patch->size(), .FileIndexToPatch = 2}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    "E_EFOBJ078.pkg",
                                    "E_EFOBJ078.pkg",
                                    87935,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "0bb063712908caa0c9089dd2d6ec4c5265efbe12"),
                                    5,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixes::E_EFOBJ078
