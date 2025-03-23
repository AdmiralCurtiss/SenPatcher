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
// Lamare roadsigns + normals
static constexpr char PatchData[] = {
#include "embed_sen3_2r30po00_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
static constexpr char PatchNormalData[] = {
#include "embed_sen3_2r30po00n_jp.h"
};
static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_R30OBJ10_O_R30OBJ11_pkg {
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
                                    "data/asset/D3D11/O_R30OBJ10.pkg",
                                    "data/asset/D3D11_us/O_R30OBJ10.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "8f6021971ead2cedd3d9bcee15e7a363acdacd9a"),
                                    1383866,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "78db8659bb52c490e88d294281097d233ee446b9"),
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
                                    "data/asset/D3D11/O_R30OBJ10.pkg",
                                    "data/asset/D3D11/O_R30OBJ10.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "8f6021971ead2cedd3d9bcee15e7a363acdacd9a"),
                                    1383866,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "78db8659bb52c490e88d294281097d233ee446b9"),
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
                                    "data/asset/D3D11/O_R30OBJ11.pkg",
                                    "data/asset/D3D11_us/O_R30OBJ11.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "33b75193023d210a0d2f9843ab5075e55a2bb31a"),
                                    1421363,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e37877cbe6375fe43bd60c6ccabcbab5146e184e"),
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
                                    "data/asset/D3D11/O_R30OBJ11.pkg",
                                    "data/asset/D3D11/O_R30OBJ11.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "33b75193023d210a0d2f9843ab5075e55a2bb31a"),
                                    1421363,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e37877cbe6375fe43bd60c6ccabcbab5146e184e"),
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
} // namespace SenLib::Sen3::FileFixes::O_R30OBJ10_O_R30OBJ11_pkg
