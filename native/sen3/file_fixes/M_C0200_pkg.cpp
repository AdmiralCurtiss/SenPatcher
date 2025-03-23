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
// Crossbell area map sign in Central Square
static constexpr char PatchData[] = {
#include "embed_sen3_8c20ce_sign_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::M_C0200_pkg {
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
                                    "data/asset/D3D11/M_C0200.pkg",
                                    "data/asset/D3D11_us/M_C0200.pkg",
                                    10648,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "757f3b0d63ac0ae5185705f9edb23fba6e94c28f"),
                                    72340799,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "cdaceaf92ad06a44566b87e8599c444f187d5bf5"),
                                    95,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{{SingleFilePatchInfo{
                .PatchData = patch->data(), .PatchLength = patch->size(), .FileIndexToPatch = 6}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_C0200.pkg",
                                    "data/asset/D3D11/M_C0200.pkg",
                                    10648,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "757f3b0d63ac0ae5185705f9edb23fba6e94c28f"),
                                    72340799,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "cdaceaf92ad06a44566b87e8599c444f187d5bf5"),
                                    95,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_C0200_pkg
