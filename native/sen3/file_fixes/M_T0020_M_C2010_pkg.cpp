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
// East Erebonia map used in Leeves and Heimdallr train stations
static constexpr char PatchDataMenu[] = {
#include "embed_sen3_9c2010menu_jp.h"
};
static constexpr size_t PatchLengthMenu = sizeof(PatchDataMenu);

// Heimdallr train station signs + normals
static constexpr char PatchDataStname[] = {
#include "embed_sen3_9c2010stname_jp.h"
};
static constexpr size_t PatchLengthStname = sizeof(PatchDataStname);
static constexpr char PatchDataStnameNormal[] = {
#include "embed_sen3_9c2010stname_n_jp.h"
};
static constexpr size_t PatchLengthStnameNormal = sizeof(PatchDataStnameNormal);
} // namespace

namespace SenLib::Sen3::FileFixes::M_T0020_M_C2010_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        auto patchMenu = DecompressFromBuffer(PatchDataMenu, PatchLengthMenu);
        if (!patchMenu) {
            return false;
        }
        auto patchStname = DecompressFromBuffer(PatchDataStname, PatchLengthStname);
        if (!patchStname) {
            return false;
        }
        auto patchStnameNormal =
            DecompressFromBuffer(PatchDataStnameNormal, PatchLengthStnameNormal);
        if (!patchStnameNormal) {
            return false;
        }

        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_C2010.pkg",
                                    "data/asset/D3D11_us/M_C2010.pkg",
                                    6952,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a0a29fb81c893963c6d25a7590991388797fd231"),
                                    28726958,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "80e9c74687b522cef1cdb65a165945de1c95b00c"),
                                    62,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 3> patches{
                {SingleFilePatchInfo{.PatchData = patchMenu->data(),
                                     .PatchLength = patchMenu->size(),
                                     .FileIndexToPatch = 22},
                 SingleFilePatchInfo{.PatchData = patchStname->data(),
                                     .PatchLength = patchStname->size(),
                                     .FileIndexToPatch = 36},
                 SingleFilePatchInfo{.PatchData = patchStnameNormal->data(),
                                     .PatchLength = patchStnameNormal->size(),
                                     .FileIndexToPatch = 37}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_C2010.pkg",
                                    "data/asset/D3D11/M_C2010.pkg",
                                    6952,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a0a29fb81c893963c6d25a7590991388797fd231"),
                                    28726958,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "80e9c74687b522cef1cdb65a165945de1c95b00c"),
                                    62,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0020.pkg",
                                    "data/asset/D3D11_us/M_T0020.pkg",
                                    13896,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "6d0ede3152631e6f46a5df07e474eca40bca71e8"),
                                    48655893,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "8ba31ba4551b31fe82359eb75083d134e1e07836"),
                                    124,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 3> patches{
                {SingleFilePatchInfo{.PatchData = patchMenu->data(),
                                     .PatchLength = patchMenu->size(),
                                     .FileIndexToPatch = 31},
                 SingleFilePatchInfo{.PatchData = patchStname->data(),
                                     .PatchLength = patchStname->size(),
                                     .FileIndexToPatch = 38},
                 SingleFilePatchInfo{.PatchData = patchStnameNormal->data(),
                                     .PatchLength = patchStnameNormal->size(),
                                     .FileIndexToPatch = 39}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0020.pkg",
                                    "data/asset/D3D11/M_T0020.pkg",
                                    13896,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "6d0ede3152631e6f46a5df07e474eca40bca71e8"),
                                    48655893,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "8ba31ba4551b31fe82359eb75083d134e1e07836"),
                                    124,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_T0020_M_C2010_pkg
