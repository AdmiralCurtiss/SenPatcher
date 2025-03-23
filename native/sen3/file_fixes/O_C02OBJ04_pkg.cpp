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
// rega shop sign + normal map
// used in Times Dept. Store in Crossbell
static constexpr char PatchData[] = {
#include "embed_sen3_9_c02obj04_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
static constexpr char PatchNormalData[] = {
#include "embed_sen3_9_c02obj04_n_jp.h"
};
static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_C02OBJ04_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }
        auto patchNormal = DecompressFromBuffer(PatchNormalData, PatchNormalLength);
        if (!patchNormal) {
            return false;
        }

        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_C02OBJ04.pkg",
                                    "data/asset/D3D11_us/O_C02OBJ04.pkg",
                                    1016,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "7e6414e487b987a5ab76652b63ff0c7d9c378c7b"),
                                    3078364,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "d2cb3534c3760006b3f0c9c93760d65bd2e06d36"),
                                    9,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patch->data(),
                                     .PatchLength = patch->size(),
                                     .FileIndexToPatch = 0},
                 SingleFilePatchInfo{.PatchData = patchNormal->data(),
                                     .PatchLength = patchNormal->size(),
                                     .FileIndexToPatch = 1}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_C02OBJ04.pkg",
                                    "data/asset/D3D11/O_C02OBJ04.pkg",
                                    1016,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "7e6414e487b987a5ab76652b63ff0c7d9c378c7b"),
                                    3078364,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "d2cb3534c3760006b3f0c9c93760d65bd2e06d36"),
                                    9,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::O_C02OBJ04_pkg
