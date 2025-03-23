#include <array>
#include <span>
#include <string_view>
#include <vector>

#include "sen/decompress_helper.h"
#include "sen/file_getter.h"
#include "sen3/file_fixes.h"
#include "sen3/patch_pkg.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_8_M_C0000[] = "Add missing Japanese environment textures";
}

namespace {
// Crossbell area map sign near Station
static constexpr char PatchData[] = {
#include "embed_sen3_p_8c00st_sign_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::M_C0000_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }

        // US directory must get an unmodified copy so that it doesn't end up with the JP texture.
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_C0000.pkg",
                                    "data/asset/D3D11_us/M_C0000.pkg",
                                    12440,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "3d58a47cede25249a513fc33f85260082a67ea0d"),
                                    89031514,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a0ecaa2a5184152fd8dbc46b91f4ff3e317c74a2"),
                                    111,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{
                {SingleFilePatchInfo{.PatchData = patch->data(),
                                     .PatchLength = patch->size(),
                                     .FileIndexToPatch = 110}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_C0000.pkg",
                                    "data/asset/D3D11/M_C0000.pkg",
                                    12440,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "3d58a47cede25249a513fc33f85260082a67ea0d"),
                                    89031514,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a0ecaa2a5184152fd8dbc46b91f4ff3e317c74a2"),
                                    111,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_C0000_pkg
