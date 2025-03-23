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
// Student Store sign
static constexpr char PatchData[] = {
#include "embed_sen3_1t02obj07_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_T02OBJ07_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }

        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_T02OBJ07.pkg",
                                    "data/asset/D3D11_us/O_T02OBJ07.pkg",
                                    456,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "fa3e9e2030d6e042e21c4e983c392db0e8d60bb0"),
                                    462039,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "38d68b5345ed78da3d90aab757faf180246dbaf1"),
                                    4,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{{SingleFilePatchInfo{
                .PatchData = patch->data(), .PatchLength = patch->size(), .FileIndexToPatch = 0}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_T02OBJ07.pkg",
                                    "data/asset/D3D11/O_T02OBJ07.pkg",
                                    456,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "fa3e9e2030d6e042e21c4e983c392db0e8d60bb0"),
                                    462039,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "38d68b5345ed78da3d90aab757faf180246dbaf1"),
                                    4,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::O_T02OBJ07_pkg
