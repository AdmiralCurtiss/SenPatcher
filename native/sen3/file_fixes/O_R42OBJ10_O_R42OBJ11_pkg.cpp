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
// Heimdallr roadsigns
static constexpr char PatchData[] = {
#include "embed_sen3_2r42po00_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
// the EN version uses the JP normals here, there is no EN normals... maybe we'll make some
// static constexpr char PatchNormalData[] = {
// #include "embed_sen3_2r42po00n_jp.h"
// };
// static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_R42OBJ10_O_R42OBJ11_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }
        // auto patchNormal = DecompressFromBuffer(PatchNormalData, PatchNormalLength);
        // if (!patchNormal) {
        //     return false;
        // }

        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_R42OBJ10.pkg",
                                    "data/asset/D3D11_us/O_R42OBJ10.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "d1ee7bd1694007339643b6ac401953a509e580de"),
                                    2388145,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "357df4ef50c264cf27d145be6a035c51476c09a9"),
                                    5,
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
                                    "data/asset/D3D11/O_R42OBJ10.pkg",
                                    "data/asset/D3D11/O_R42OBJ10.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "d1ee7bd1694007339643b6ac401953a509e580de"),
                                    2388145,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "357df4ef50c264cf27d145be6a035c51476c09a9"),
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
                                    "data/asset/D3D11/O_R42OBJ11.pkg",
                                    "data/asset/D3D11_us/O_R42OBJ11.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a7a233578b65a45e229b6bd0e47f356b91fdb7b3"),
                                    2392821,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "5f8a6ff167247c517ae90a7b3146454ad9fe9d7b"),
                                    5,
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
                                    "data/asset/D3D11/O_R42OBJ11.pkg",
                                    "data/asset/D3D11/O_R42OBJ11.pkg",
                                    568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a7a233578b65a45e229b6bd0e47f356b91fdb7b3"),
                                    2392821,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "5f8a6ff167247c517ae90a7b3146454ad9fe9d7b"),
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
} // namespace SenLib::Sen3::FileFixes::O_R42OBJ10_O_R42OBJ11_pkg
