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
// Saint-Arkh airport sign + normals
static constexpr char PatchData[] = {
#include "embed_sen3_7t10obj02_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
static constexpr char PatchNormalData[] = {
#include "embed_sen3_7t10obj02n_jp.h"
};
static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_T10OBJ02_pkg {
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
                                    "data/asset/D3D11/O_T10OBJ02.pkg",
                                    "data/asset/D3D11_us/O_T10OBJ02.pkg",
                                    1464,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a1f3dd05ad1dabc5017c7bf19170c33691fa8794"),
                                    2301413,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a68da9f2e551e0898cf96973ea1de8a67984e3b3"),
                                    13,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patch->data(),
                                     .PatchLength = patch->size(),
                                     .FileIndexToPatch = 3},
                 SingleFilePatchInfo{.PatchData = patchNormal->data(),
                                     .PatchLength = patchNormal->size(),
                                     .FileIndexToPatch = 4}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_T10OBJ02.pkg",
                                    "data/asset/D3D11/O_T10OBJ02.pkg",
                                    1464,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a1f3dd05ad1dabc5017c7bf19170c33691fa8794"),
                                    2301413,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a68da9f2e551e0898cf96973ea1de8a67984e3b3"),
                                    13,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::O_T10OBJ02_pkg
