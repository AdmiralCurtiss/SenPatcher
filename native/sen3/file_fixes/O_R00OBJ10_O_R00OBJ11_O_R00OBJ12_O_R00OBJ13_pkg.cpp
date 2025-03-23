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
// Sutherland roadsigns + normals
static constexpr char PatchData[] = {
#include "embed_sen3_2r00po00_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
static constexpr char PatchNormalData[] = {
#include "embed_sen3_2r00po00n_jp.h"
};
static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_R00OBJ10_O_R00OBJ11_O_R00OBJ12_O_R00OBJ13_pkg {
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
                                    "data/asset/D3D11/O_R00OBJ10.pkg",
                                    "data/asset/D3D11_us/O_R00OBJ10.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "78353a07c7d4d26e71851904bc03a9da0ddc12fc"),
                                    1473356,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "fbdb0cb7269de25091191a3146e62f406301d501"),
                                    5,
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
                                    "data/asset/D3D11/O_R00OBJ10.pkg",
                                    "data/asset/D3D11/O_R00OBJ10.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "78353a07c7d4d26e71851904bc03a9da0ddc12fc"),
                                    1473356,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "fbdb0cb7269de25091191a3146e62f406301d501"),
                                    5,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_R00OBJ11.pkg",
                                    "data/asset/D3D11_us/O_R00OBJ11.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "08516392a42d48e44bc1450f65d73b7ca74d6bf5"),
                                    1499971,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "36a64ca7fe304f5aa29f7a65a270dee1da8b781b"),
                                    5,
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
                                    "data/asset/D3D11/O_R00OBJ11.pkg",
                                    "data/asset/D3D11/O_R00OBJ11.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "08516392a42d48e44bc1450f65d73b7ca74d6bf5"),
                                    1499971,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "36a64ca7fe304f5aa29f7a65a270dee1da8b781b"),
                                    5,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_R00OBJ12.pkg",
                                    "data/asset/D3D11_us/O_R00OBJ12.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "47dea78e6c8c2a758ca4a0a76e5178e31fd9c53e"),
                                    1462466,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "4ec912310c9c97794c21eab85548cca4136dcdd3"),
                                    5,
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
                                    "data/asset/D3D11/O_R00OBJ12.pkg",
                                    "data/asset/D3D11/O_R00OBJ12.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "47dea78e6c8c2a758ca4a0a76e5178e31fd9c53e"),
                                    1462466,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "4ec912310c9c97794c21eab85548cca4136dcdd3"),
                                    5,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_R00OBJ13.pkg",
                                    "data/asset/D3D11_us/O_R00OBJ13.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "076fc41b0bdb62aa6d43fe3f0a8dcecc47b73d9e"),
                                    1462475,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "5c8a388e7931dffd0f2fa8df014a5c14fe198a7c"),
                                    5,
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
                                    "data/asset/D3D11/O_R00OBJ13.pkg",
                                    "data/asset/D3D11/O_R00OBJ13.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "076fc41b0bdb62aa6d43fe3f0a8dcecc47b73d9e"),
                                    1462475,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "5c8a388e7931dffd0f2fa8df014a5c14fe198a7c"),
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
} // namespace SenLib::Sen3::FileFixes::O_R00OBJ10_O_R00OBJ11_O_R00OBJ12_O_R00OBJ13_pkg
