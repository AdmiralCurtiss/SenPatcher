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
__declspec(dllexport) char SenPatcherFix_8_M_T0010[] = "Fix dorm room sign for Valerie.";
}

namespace {
// Character name signs
static constexpr char PatchDataRoomSignEn[] = {
#include "embed_sen3_sign09.h"
};
static constexpr size_t PatchLengthRoomSignEn = sizeof(PatchDataRoomSignEn);
static constexpr char PatchDataRoomSignJp[] = {
#include "embed_sen3_sign09_jp.h"
};
static constexpr size_t PatchLengthRoomSignJp = sizeof(PatchDataRoomSignJp);

// "Branch Campus Dormitory" sign + corresponding normal map
static constexpr char PatchDataDormSign[] = {
#include "embed_sen3_1t00hou14_jp.h"
};
static constexpr size_t PatchLengthDormSign = sizeof(PatchDataDormSign);
static constexpr char PatchDataDormSignNormal[] = {
#include "embed_sen3_1t00hou14_n_jp.h"
};
static constexpr size_t PatchLengthDormSignNormal = sizeof(PatchDataDormSignNormal);
} // namespace

namespace SenLib::Sen3::FileFixes::M_T0010_M_A2003_O_T00HOU01_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        auto patchRoomSignEn = DecompressFromBuffer(PatchDataRoomSignEn, PatchLengthRoomSignEn);
        if (!patchRoomSignEn) {
            return false;
        }
        auto patchRoomSignJp = DecompressFromBuffer(PatchDataRoomSignJp, PatchLengthRoomSignJp);
        if (!patchRoomSignJp) {
            return false;
        }
        auto patchDormSignJp = DecompressFromBuffer(PatchDataDormSign, PatchLengthDormSign);
        if (!patchDormSignJp) {
            return false;
        }
        auto patchDormSignNormalJp =
            DecompressFromBuffer(PatchDataDormSignNormal, PatchLengthDormSignNormal);
        if (!patchDormSignNormalJp) {
            return false;
        }

        {
            std::array<SingleFilePatchInfo, 1> patches{
                {SingleFilePatchInfo{.PatchData = patchRoomSignEn->data(),
                                     .PatchLength = patchRoomSignEn->size(),
                                     .FileIndexToPatch = 76}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0010.pkg",
                                    "data/asset/D3D11_us/M_T0010.pkg",
                                    46040,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "2dcafdbfe0c5954f797653cd442fbe8a4fa2c80d"),
                                    179678858,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e6ac286949f5b408d28b2ee26fae62aad981db2f"),
                                    411,
                                    patches)) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 3> patches{
                {SingleFilePatchInfo{.PatchData = patchRoomSignJp->data(),
                                     .PatchLength = patchRoomSignJp->size(),
                                     .FileIndexToPatch = 76},
                 SingleFilePatchInfo{.PatchData = patchDormSignJp->data(),
                                     .PatchLength = patchDormSignJp->size(),
                                     .FileIndexToPatch = 96},
                 SingleFilePatchInfo{.PatchData = patchDormSignNormalJp->data(),
                                     .PatchLength = patchDormSignNormalJp->size(),
                                     .FileIndexToPatch = 97}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0010.pkg",
                                    "data/asset/D3D11/M_T0010.pkg",
                                    46040,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "2dcafdbfe0c5954f797653cd442fbe8a4fa2c80d"),
                                    179678858,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e6ac286949f5b408d28b2ee26fae62aad981db2f"),
                                    411,
                                    patches)) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{
                {SingleFilePatchInfo{.PatchData = patchRoomSignEn->data(),
                                     .PatchLength = patchRoomSignEn->size(),
                                     .FileIndexToPatch = 14}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_A2003.pkg",
                                    "data/asset/D3D11_us/M_A2003.pkg",
                                    13224,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "98d405f214340ba7210fc7bf5dfb6151c560aaa7"),
                                    59675703,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "43ffeaaf730770cf46a43783bcbf4e6787a9bb76"),
                                    118,
                                    patches)) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 1> patches{
                {SingleFilePatchInfo{.PatchData = patchRoomSignJp->data(),
                                     .PatchLength = patchRoomSignJp->size(),
                                     .FileIndexToPatch = 14}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_A2003.pkg",
                                    "data/asset/D3D11/M_A2003.pkg",
                                    13224,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "98d405f214340ba7210fc7bf5dfb6151c560aaa7"),
                                    59675703,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "43ffeaaf730770cf46a43783bcbf4e6787a9bb76"),
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
                                    "data/asset/D3D11/O_T00HOU01.pkg",
                                    "data/asset/D3D11_us/O_T00HOU01.pkg",
                                    4936,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e8e3204f620849442af5495cf019d344a305a01a"),
                                    15340301,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "eb00c38ae657d2e86c15150088c6032fb2925252"),
                                    44,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patchDormSignJp->data(),
                                     .PatchLength = patchDormSignJp->size(),
                                     .FileIndexToPatch = 22},
                 SingleFilePatchInfo{.PatchData = patchDormSignNormalJp->data(),
                                     .PatchLength = patchDormSignNormalJp->size(),
                                     .FileIndexToPatch = 23}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/O_T00HOU01.pkg",
                                    "data/asset/D3D11/O_T00HOU01.pkg",
                                    4936,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e8e3204f620849442af5495cf019d344a305a01a"),
                                    15340301,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "eb00c38ae657d2e86c15150088c6032fb2925252"),
                                    44,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_T0010_M_A2003_O_T00HOU01_pkg
