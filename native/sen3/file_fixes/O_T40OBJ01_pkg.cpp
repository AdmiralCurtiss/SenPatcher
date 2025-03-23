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
// "Welcome to Raquel" sign
static constexpr char PatchData[] = {
#include "embed_sen3_0sign018_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_T40OBJ01_pkg {
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
                                    "data/asset/D3D11/O_T40OBJ01.pkg",
                                    "data/asset/D3D11_us/O_T40OBJ01.pkg",
                                    680,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "060a3e1d0b10a57b9666a5898730215f2535211b"),
                                    416678,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "b2af68aba334c89e045eba95e43e87f00a2b9bac"),
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
                                    "data/asset/D3D11/O_T40OBJ01.pkg",
                                    "data/asset/D3D11/O_T40OBJ01.pkg",
                                    680,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "060a3e1d0b10a57b9666a5898730215f2535211b"),
                                    416678,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "b2af68aba334c89e045eba95e43e87f00a2b9bac"),
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
} // namespace SenLib::Sen3::FileFixes::O_T40OBJ01_pkg
