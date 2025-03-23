#include <array>
#include <span>
#include <string_view>
#include <vector>

#include "sen/decompress_helper.h"
#include "sen/file_getter.h"
#include "sen3/file_fixes.h"
#include "sen3/patch_pkg.h"
#include "util/hash/sha1.h"

namespace {
// Crossbell + Heimdallr maps for field exercise briefings
static constexpr char PatchDataB[] = {
#include "embed_sen3_0view022b_jp.h"
};
static constexpr size_t PatchLengthB = sizeof(PatchDataB);
static constexpr char PatchDataD[] = {
#include "embed_sen3_0view022d_jp.h"
};
static constexpr size_t PatchLengthD = sizeof(PatchDataD);
} // namespace

namespace SenLib::Sen3::FileFixes::O_S50OBJ11_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        auto patchB = DecompressFromBuffer(PatchDataB, PatchLengthB);
        if (!patchB) {
            return false;
        }
        auto patchD = DecompressFromBuffer(PatchDataD, PatchLengthD);
        if (!patchD) {
            return false;
        }

        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_S50OBJ11.pkg",
                                    "data/asset/D3D11_us/O_S50OBJ11.pkg",
                                    2472,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "db9c3944869a4d93e925ea63ee00742ff8bfe50c"),
                                    10552274,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "db9d1304efcff62cb8c119c4624dbbaf7ff6344e"),
                                    22,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patchB->data(),
                                     .PatchLength = patchB->size(),
                                     .FileIndexToPatch = 4},
                 SingleFilePatchInfo{.PatchData = patchD->data(),
                                     .PatchLength = patchD->size(),
                                     .FileIndexToPatch = 6}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_S50OBJ11.pkg",
                                    "data/asset/D3D11/O_S50OBJ11.pkg",
                                    2472,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "db9c3944869a4d93e925ea63ee00742ff8bfe50c"),
                                    10552274,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "db9d1304efcff62cb8c119c4624dbbaf7ff6344e"),
                                    22,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::O_S50OBJ11_pkg
