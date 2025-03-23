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
// Saint-Arkh station sign + normal map
static constexpr char PatchData[] = {
#include "embed_sen3_7t12hou28_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
static constexpr char PatchNormalData[] = {
#include "embed_sen3_7t12hou28n_jp.h"
};
static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::M_T1000_pkg {
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
                                    "data/asset/D3D11/M_T1000.pkg",
                                    "data/asset/D3D11_us/M_T1000.pkg",
                                    8744,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "98536f998c1b27a92fa62bd4dd5fc8c5bc3c8bbf"),
                                    84003545,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "c185fde97cdde53a57576228be0d0d599ff89965"),
                                    78,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patch->data(),
                                     .PatchLength = patch->size(),
                                     .FileIndexToPatch = 28},
                 SingleFilePatchInfo{.PatchData = patchNormal->data(),
                                     .PatchLength = patchNormal->size(),
                                     .FileIndexToPatch = 29}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T1000.pkg",
                                    "data/asset/D3D11/M_T1000.pkg",
                                    8744,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "98536f998c1b27a92fa62bd4dd5fc8c5bc3c8bbf"),
                                    84003545,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "c185fde97cdde53a57576228be0d0d599ff89965"),
                                    78,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_T1000_pkg
