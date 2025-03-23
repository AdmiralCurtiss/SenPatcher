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
// Saint-Arkh station plaque + normals
static constexpr char PatchData[] = {
#include "embed_sen3_7t10obj05_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
static constexpr char PatchNormalData[] = {
#include "embed_sen3_7t10obj05n_jp.h"
};
static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_T10OBJ05_pkg {
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
                                    "data/asset/D3D11/O_T10OBJ05.pkg",
                                    "data/asset/D3D11_us/O_T10OBJ05.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "d11d109fddf05327b3e42e08294c526799324a4c"),
                                    1701406,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e13c530180e0920405345782d8ef442202f63194"),
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
                                    "data/asset/D3D11/O_T10OBJ05.pkg",
                                    "data/asset/D3D11/O_T10OBJ05.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "d11d109fddf05327b3e42e08294c526799324a4c"),
                                    1701406,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e13c530180e0920405345782d8ef442202f63194"),
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
} // namespace SenLib::Sen3::FileFixes::O_T10OBJ05_pkg
