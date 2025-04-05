#include <array>
#include <span>
#include <string_view>
#include <vector>

#include "sen/decompress_helper.h"
#include "sen/file_getter.h"
#include "tx/file_fixes.h"
#include "tx/patch_pkg.h"
#include "util/hash/sha1.h"

namespace {
static constexpr char PatchData[] = {
#include "embed_tx_efobj079a_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::TX::FileFixes::E_EFOBJ079 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }

        {
            std::array<SingleFilePatchInfo, 1> patches{{SingleFilePatchInfo{
                .PatchData = patch->data(), .PatchLength = patch->size(), .FileIndexToPatch = 2}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    "E_EFOBJ079.pkg",
                                    "E_EFOBJ079.pkg",
                                    90531,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "7644c32e606bea9b369ba065e9cca8e809d5d307"),
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
} // namespace SenLib::TX::FileFixes::E_EFOBJ079
