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
// RMP insignia + corresponding normal map
// used in cutscene in RMP office in Chapter 4
static constexpr char PatchData[] = {
#include "embed_sen3_ac2020shield01_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
static constexpr char PatchNormalData[] = {
#include "embed_sen3_ac2020shield01_n_jp.h"
};
static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::M_C2020_pkg {
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
                                    "data/asset/D3D11/M_C2020.pkg",
                                    "data/asset/D3D11_us/M_C2020.pkg",
                                    4712,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "c8befc71270bdcc85d1dcc16337a2df8d9d82105"),
                                    24865290,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "6300e8e834278cf232785c7eb0c40ee046c862b6"),
                                    42,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patch->data(),
                                     .PatchLength = patch->size(),
                                     .FileIndexToPatch = 24},
                 SingleFilePatchInfo{.PatchData = patchNormal->data(),
                                     .PatchLength = patchNormal->size(),
                                     .FileIndexToPatch = 25}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_C2020.pkg",
                                    "data/asset/D3D11/M_C2020.pkg",
                                    4712,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "c8befc71270bdcc85d1dcc16337a2df8d9d82105"),
                                    24865290,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "6300e8e834278cf232785c7eb0c40ee046c862b6"),
                                    42,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_C2020_pkg
