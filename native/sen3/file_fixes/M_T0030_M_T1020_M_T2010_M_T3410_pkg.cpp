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
// Some kind of diploma with keyboard smashing text, EN version just blurs this so it's unreadable
// Used in Leeves church
static constexpr char PatchData[] = {
#include "embed_sen3_at1020komono04_jp.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::M_T0030_M_T1020_M_T2010_M_T3410_pkg {
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
                                    "data/asset/D3D11/M_T0030.pkg",
                                    "data/asset/D3D11_us/M_T0030.pkg",
                                    13224,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "bd4891a48f3ef5a523b208b33f1ca329362e5ba5"),
                                    85627162,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "19ab12805eb3f807cba2a1bc3ba6a13e7c1b0917"),
                                    118,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{{SingleFilePatchInfo{
                .PatchData = patch->data(), .PatchLength = patch->size(), .FileIndexToPatch = 80}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0030.pkg",
                                    "data/asset/D3D11/M_T0030.pkg",
                                    13224,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "bd4891a48f3ef5a523b208b33f1ca329362e5ba5"),
                                    85627162,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "19ab12805eb3f807cba2a1bc3ba6a13e7c1b0917"),
                                    118,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T1020.pkg",
                                    "data/asset/D3D11_us/M_T1020.pkg",
                                    14904,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "631527f8d82725dcfa64b4d456a4f8350ffcff10"),
                                    97571230,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "61e705fe76c215d037f6c188f248e9c02db293e2"),
                                    133,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{{SingleFilePatchInfo{
                .PatchData = patch->data(), .PatchLength = patch->size(), .FileIndexToPatch = 68}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T1020.pkg",
                                    "data/asset/D3D11/M_T1020.pkg",
                                    14904,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "631527f8d82725dcfa64b4d456a4f8350ffcff10"),
                                    97571230,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "61e705fe76c215d037f6c188f248e9c02db293e2"),
                                    133,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T2010.pkg",
                                    "data/asset/D3D11_us/M_T2010.pkg",
                                    13448,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "d5aed1ffc984f2da434ac20cc2d71806948d5f89"),
                                    83616263,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "9427d420d0b6dc5d2b13f6ad47117d4adf0de615"),
                                    120,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{{SingleFilePatchInfo{
                .PatchData = patch->data(), .PatchLength = patch->size(), .FileIndexToPatch = 80}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T2010.pkg",
                                    "data/asset/D3D11/M_T2010.pkg",
                                    13448,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "d5aed1ffc984f2da434ac20cc2d71806948d5f89"),
                                    83616263,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "9427d420d0b6dc5d2b13f6ad47117d4adf0de615"),
                                    120,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T3410.pkg",
                                    "data/asset/D3D11_us/M_T3410.pkg",
                                    14568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "70f198bf6233039e3a012ae406a93ca74ffd0f0e"),
                                    90631094,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "505fd70bb918a0f0c78bc07b6a450a16516ffa9a"),
                                    130,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{{SingleFilePatchInfo{
                .PatchData = patch->data(), .PatchLength = patch->size(), .FileIndexToPatch = 69}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T3410.pkg",
                                    "data/asset/D3D11/M_T3410.pkg",
                                    14568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "70f198bf6233039e3a012ae406a93ca74ffd0f0e"),
                                    90631094,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "505fd70bb918a0f0c78bc07b6a450a16516ffa9a"),
                                    130,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_T0030_M_T1020_M_T2010_M_T3410_pkg
