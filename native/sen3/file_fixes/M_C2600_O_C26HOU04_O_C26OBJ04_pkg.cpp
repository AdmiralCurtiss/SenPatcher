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
// Vesta Street sign + corresponding normal map
static constexpr char PatchData[] = {
#include "embed_sen3_3c26ob04_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
static constexpr char PatchNormalData[] = {
#include "embed_sen3_3c26ob04n_jp.h"
};
static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::M_C2600_O_C26HOU04_O_C26OBJ04_pkg {
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
                                    "data/asset/D3D11/M_C2600.pkg",
                                    "data/asset/D3D11_us/M_C2600.pkg",
                                    5496,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "94478bf8742b9f0064d717b647284887ba47784e"),
                                    17084215,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "c67cef00f345f2ac2f5f654bba9598557336d2bb"),
                                    49,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patch->data(),
                                     .PatchLength = patch->size(),
                                     .FileIndexToPatch = 26},
                 SingleFilePatchInfo{.PatchData = patchNormal->data(),
                                     .PatchLength = patchNormal->size(),
                                     .FileIndexToPatch = 27}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_C2600.pkg",
                                    "data/asset/D3D11/M_C2600.pkg",
                                    5496,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "94478bf8742b9f0064d717b647284887ba47784e"),
                                    17084215,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "c67cef00f345f2ac2f5f654bba9598557336d2bb"),
                                    49,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_C26HOU04.pkg",
                                    "data/asset/D3D11_us/O_C26HOU04.pkg",
                                    4488,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "9e5e40f7d426e13f4ad085d30628fd4a700a5850"),
                                    12172319,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "4e37e14cd1768a5eee0592e6d35103528bf26609"),
                                    40,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patch->data(),
                                     .PatchLength = patch->size(),
                                     .FileIndexToPatch = 21},
                 SingleFilePatchInfo{.PatchData = patchNormal->data(),
                                     .PatchLength = patchNormal->size(),
                                     .FileIndexToPatch = 22}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_C26HOU04.pkg",
                                    "data/asset/D3D11/O_C26HOU04.pkg",
                                    4488,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "9e5e40f7d426e13f4ad085d30628fd4a700a5850"),
                                    12172319,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "4e37e14cd1768a5eee0592e6d35103528bf26609"),
                                    40,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_C26OBJ04.pkg",
                                    "data/asset/D3D11_us/O_C26OBJ04.pkg",
                                    3144,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "f0e591c0cecf5910cd9c49ae6075ae5c227fbe79"),
                                    7065574,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "9c5692f42602871ea92bdccf21d2c8e9891273b7"),
                                    28,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patch->data(),
                                     .PatchLength = patch->size(),
                                     .FileIndexToPatch = 10},
                 SingleFilePatchInfo{.PatchData = patchNormal->data(),
                                     .PatchLength = patchNormal->size(),
                                     .FileIndexToPatch = 11}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_C26OBJ04.pkg",
                                    "data/asset/D3D11/O_C26OBJ04.pkg",
                                    3144,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "f0e591c0cecf5910cd9c49ae6075ae5c227fbe79"),
                                    7065574,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "9c5692f42602871ea92bdccf21d2c8e9891273b7"),
                                    28,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_C2600_O_C26HOU04_O_C26OBJ04_pkg
