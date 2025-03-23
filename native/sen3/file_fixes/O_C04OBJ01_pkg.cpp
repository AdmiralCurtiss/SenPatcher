#include <string_view>
#include <vector>

#include "modload/loaded_pka.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen/decompress_helper.h"
#include "sen/file_getter.h"
#include "sen3/file_fixes.h"
#include "sen3/patch_pkg.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_8_O_C04OBJ01[] =
    "Fix Business Owners' Association sign texture.";
}

namespace {
static constexpr char PatchDataEn[] = {
#include "embed_sen3_0sign003.h"
};
static constexpr size_t PatchLengthEn = sizeof(PatchDataEn);
static constexpr char PatchDataJp[] = {
#include "embed_sen3_0sign003_jp.h"
};
static constexpr size_t PatchLengthJp = sizeof(PatchDataJp);
} // namespace

namespace SenLib::Sen3::FileFixes::O_C04OBJ01_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        auto patchEn = DecompressFromBuffer(PatchDataEn, PatchLengthEn);
        if (!patchEn) {
            return false;
        }
        auto patchJp = DecompressFromBuffer(PatchDataJp, PatchLengthJp);
        if (!patchJp) {
            return false;
        }

        {
            std::array<SingleFilePatchInfo, 1> patches{
                {SingleFilePatchInfo{.PatchData = patchEn->data(),
                                     .PatchLength = patchEn->size(),
                                     .FileIndexToPatch = 1}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_C04OBJ01.pkg",
                                    "data/asset/D3D11_us/O_C04OBJ01.pkg",
                                    680,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "ffe4149578dc62ac6d95a7889adf58ff05966bde"),
                                    215278,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "33519219ed647067bc8a52a9e2ebc850cb9821fd"),
                                    6,
                                    patches)) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{
                {SingleFilePatchInfo{.PatchData = patchJp->data(),
                                     .PatchLength = patchJp->size(),
                                     .FileIndexToPatch = 1}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_C04OBJ01.pkg",
                                    "data/asset/D3D11/O_C04OBJ01.pkg",
                                    680,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "ffe4149578dc62ac6d95a7889adf58ff05966bde"),
                                    215278,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "33519219ed647067bc8a52a9e2ebc850cb9821fd"),
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
} // namespace SenLib::Sen3::FileFixes::O_C04OBJ01_pkg
