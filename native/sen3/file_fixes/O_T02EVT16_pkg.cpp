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
// "War of the Lions" classroom lecture blackboard writing
static constexpr char PatchData[] = {
#include "embed_sen3_1t02evt16_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_T02EVT16_pkg {
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
                                    "data/asset/D3D11/O_T02EVT16.pkg",
                                    "data/asset/D3D11_us/O_T02EVT16.pkg",
                                    904,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "60527c4ca547114663c49dc5ad8227d7871032ea"),
                                    1159896,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "542a1b72c4bf9571e9fc7a73a23637001a3645c3"),
                                    8,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{{SingleFilePatchInfo{
                .PatchData = patch->data(), .PatchLength = patch->size(), .FileIndexToPatch = 1}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_T02EVT16.pkg",
                                    "data/asset/D3D11/O_T02EVT16.pkg",
                                    904,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "60527c4ca547114663c49dc5ad8227d7871032ea"),
                                    1159896,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "542a1b72c4bf9571e9fc7a73a23637001a3645c3"),
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
} // namespace SenLib::Sen3::FileFixes::O_T02EVT16_pkg
