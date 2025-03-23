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
// Hamel Village sign
static constexpr char PatchData[] = {
#include "embed_sen3_7r14obj01_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_R14OBJ01_pkg {
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
                                    "data/asset/D3D11/O_R14OBJ01.pkg",
                                    "data/asset/D3D11_us/O_R14OBJ01.pkg",
                                    680,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "69b02d6c70c1cde4a5e2e62c2cd67651688dfe0b"),
                                    3255043,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "af96568dd16be830ff371b3f800025ff3d09b3dd"),
                                    6,
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
                                    "data/asset/D3D11/O_R14OBJ01.pkg",
                                    "data/asset/D3D11/O_R14OBJ01.pkg",
                                    680,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "69b02d6c70c1cde4a5e2e62c2cd67651688dfe0b"),
                                    3255043,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "af96568dd16be830ff371b3f800025ff3d09b3dd"),
                                    6,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::O_R14OBJ01_pkg
