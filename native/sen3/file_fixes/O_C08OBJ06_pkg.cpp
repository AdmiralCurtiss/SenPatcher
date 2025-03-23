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
// Crossbell area map sign in Harbor District
static constexpr char PatchData[] = {
#include "embed_sen3_8c08sign05_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_C08OBJ06_pkg {
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
                                    "data/asset/D3D11/O_C08OBJ06.pkg",
                                    "data/asset/D3D11_us/O_C08OBJ06.pkg",
                                    792,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "bf15e6e504a2d3ee80b2d66e4f78d96a0a935958"),
                                    2012751,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e8da88eb0d262d39e6a9413974e4326757ba216e"),
                                    7,
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
                                    "data/asset/D3D11/O_C08OBJ06.pkg",
                                    "data/asset/D3D11/O_C08OBJ06.pkg",
                                    792,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "bf15e6e504a2d3ee80b2d66e4f78d96a0a935958"),
                                    2012751,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e8da88eb0d262d39e6a9413974e4326757ba216e"),
                                    7,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::O_C08OBJ06_pkg
