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
// Hamel Road 'closed due to landslide' sign + normals
static constexpr char PatchData[] = {
#include "embed_sen3_7r12obj00_1_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
static constexpr char PatchNormalData[] = {
#include "embed_sen3_7r12obj00_1n_jp.h"
};
static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_R12DOR00_pkg {
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
                                    "data/asset/D3D11/O_R12DOR00.pkg",
                                    "data/asset/D3D11_us/O_R12DOR00.pkg",
                                    904,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "ef761031a4be9ed9e50a40f28bd71809d800d7cc"),
                                    4819668,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "d070c9897eb480defa142e5bacb9644f6a89946b"),
                                    8,
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
                                    "data/asset/D3D11/O_R12DOR00.pkg",
                                    "data/asset/D3D11/O_R12DOR00.pkg",
                                    904,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "ef761031a4be9ed9e50a40f28bd71809d800d7cc"),
                                    4819668,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "d070c9897eb480defa142e5bacb9644f6a89946b"),
                                    8,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::O_R12DOR00_pkg
