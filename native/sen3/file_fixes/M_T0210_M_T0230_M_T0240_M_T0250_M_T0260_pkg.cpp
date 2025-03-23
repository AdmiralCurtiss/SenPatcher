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
// School room signs + normal map
static constexpr char PatchData7[] = {
#include "embed_sen3_sign07_jp.h"
};
static constexpr size_t PatchLength7 = sizeof(PatchData7);
static constexpr char PatchData7Normal[] = {
#include "embed_sen3_sign07_n_jp.h"
};
static constexpr size_t PatchLength7Normal = sizeof(PatchData7Normal);

// Class VII/VIII/IX logos + normal map
static constexpr char PatchData8[] = {
#include "embed_sen3_sign08_jp.h"
};
static constexpr size_t PatchLength8 = sizeof(PatchData8);
static constexpr char PatchData8Normal[] = {
#include "embed_sen3_sign08_n_jp.h"
};
static constexpr size_t PatchLength8Normal = sizeof(PatchData8Normal);
} // namespace

namespace SenLib::Sen3::FileFixes::M_T0210_M_T0230_M_T0240_M_T0250_M_T0260_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        auto patch7 = DecompressFromBuffer(PatchData7, PatchLength7);
        if (!patch7) {
            return false;
        }
        auto patch7Normal = DecompressFromBuffer(PatchData7Normal, PatchLength7Normal);
        if (!patch7Normal) {
            return false;
        }
        auto patch8 = DecompressFromBuffer(PatchData8, PatchLength8);
        if (!patch8) {
            return false;
        }
        auto patch8Normal = DecompressFromBuffer(PatchData8Normal, PatchLength8Normal);
        if (!patch8Normal) {
            return false;
        }

        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0210.pkg",
                                    "data/asset/D3D11_us/M_T0210.pkg",
                                    26104,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "8c628851f4343363cc476338e9ccc9703e83a15c"),
                                    99860728,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "ec75cd21c69d6f8c69d96ab139b10a07e3e11ad3"),
                                    233,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 4> patches{
                {SingleFilePatchInfo{.PatchData = patch7->data(),
                                     .PatchLength = patch7->size(),
                                     .FileIndexToPatch = 48},
                 SingleFilePatchInfo{.PatchData = patch7Normal->data(),
                                     .PatchLength = patch7Normal->size(),
                                     .FileIndexToPatch = 49},
                 SingleFilePatchInfo{.PatchData = patch8->data(),
                                     .PatchLength = patch8->size(),
                                     .FileIndexToPatch = 50},
                 SingleFilePatchInfo{.PatchData = patch8Normal->data(),
                                     .PatchLength = patch8Normal->size(),
                                     .FileIndexToPatch = 51}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0210.pkg",
                                    "data/asset/D3D11/M_T0210.pkg",
                                    26104,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "8c628851f4343363cc476338e9ccc9703e83a15c"),
                                    99860728,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "ec75cd21c69d6f8c69d96ab139b10a07e3e11ad3"),
                                    233,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0230.pkg",
                                    "data/asset/D3D11_us/M_T0230.pkg",
                                    11768,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a16e429e087c584faa3272117dc5d2bdc0c2acf4"),
                                    40317272,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "be9498b9cf7586d020a6cc91e66f5259d8b2e080"),
                                    105,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patch7->data(),
                                     .PatchLength = patch7->size(),
                                     .FileIndexToPatch = 17},
                 SingleFilePatchInfo{.PatchData = patch7Normal->data(),
                                     .PatchLength = patch7Normal->size(),
                                     .FileIndexToPatch = 18}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0230.pkg",
                                    "data/asset/D3D11/M_T0230.pkg",
                                    11768,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "a16e429e087c584faa3272117dc5d2bdc0c2acf4"),
                                    40317272,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "be9498b9cf7586d020a6cc91e66f5259d8b2e080"),
                                    105,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0240.pkg",
                                    "data/asset/D3D11_us/M_T0240.pkg",
                                    25096,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "677c286f8667a1cde9ae530e88fa6d2d8d2bc7a3"),
                                    79584442,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e064ddb03e972e9acce9e6459ca596f1a9c5f035"),
                                    224,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 4> patches{
                {SingleFilePatchInfo{.PatchData = patch7->data(),
                                     .PatchLength = patch7->size(),
                                     .FileIndexToPatch = 45},
                 SingleFilePatchInfo{.PatchData = patch7Normal->data(),
                                     .PatchLength = patch7Normal->size(),
                                     .FileIndexToPatch = 46},
                 SingleFilePatchInfo{.PatchData = patch8->data(),
                                     .PatchLength = patch8->size(),
                                     .FileIndexToPatch = 47},
                 SingleFilePatchInfo{.PatchData = patch8Normal->data(),
                                     .PatchLength = patch8Normal->size(),
                                     .FileIndexToPatch = 48}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0240.pkg",
                                    "data/asset/D3D11/M_T0240.pkg",
                                    25096,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "677c286f8667a1cde9ae530e88fa6d2d8d2bc7a3"),
                                    79584442,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "e064ddb03e972e9acce9e6459ca596f1a9c5f035"),
                                    224,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0250.pkg",
                                    "data/asset/D3D11_us/M_T0250.pkg",
                                    14568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "6481c4fbd83d079fe3b6d573392acd42cf39b609"),
                                    48089934,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "bbace4e21a210f74496601ad4fb772ab77d76196"),
                                    130,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 4> patches{
                {SingleFilePatchInfo{.PatchData = patch7->data(),
                                     .PatchLength = patch7->size(),
                                     .FileIndexToPatch = 19},
                 SingleFilePatchInfo{.PatchData = patch7Normal->data(),
                                     .PatchLength = patch7Normal->size(),
                                     .FileIndexToPatch = 20},
                 SingleFilePatchInfo{.PatchData = patch8->data(),
                                     .PatchLength = patch8->size(),
                                     .FileIndexToPatch = 21},
                 SingleFilePatchInfo{.PatchData = patch8Normal->data(),
                                     .PatchLength = patch8Normal->size(),
                                     .FileIndexToPatch = 22}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0250.pkg",
                                    "data/asset/D3D11/M_T0250.pkg",
                                    14568,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "6481c4fbd83d079fe3b6d573392acd42cf39b609"),
                                    48089934,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "bbace4e21a210f74496601ad4fb772ab77d76196"),
                                    130,
                                    patches)) {
                return false;
            }
        }
        {
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0260.pkg",
                                    "data/asset/D3D11_us/M_T0260.pkg",
                                    15800,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "68c7037eda991a42cd1c596ece049f525c9bd4f5"),
                                    70852233,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "f21bc491312db1cd7040c95790a0b44aebdbe1cd"),
                                    141,
                                    {})) {
                return false;
            }
        }
        {
            std::array<SingleFilePatchInfo, 2> patches{
                {SingleFilePatchInfo{.PatchData = patch7->data(),
                                     .PatchLength = patch7->size(),
                                     .FileIndexToPatch = 19},
                 SingleFilePatchInfo{.PatchData = patch7Normal->data(),
                                     .PatchLength = patch7Normal->size(),
                                     .FileIndexToPatch = 20}}};
            if (!TryApplyPkgPatches(getCheckedFile,
                                    result,
                                    vanillaPKAs,
                                    getCheckedFilePkaStub,
                                    "data/asset/D3D11/M_T0260.pkg",
                                    "data/asset/D3D11/M_T0260.pkg",
                                    15800,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "68c7037eda991a42cd1c596ece049f525c9bd4f5"),
                                    70852233,
                                    HyoutaUtils::Hash::SHA1FromHexString(
                                        "f21bc491312db1cd7040c95790a0b44aebdbe1cd"),
                                    141,
                                    patches)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_T0210_M_T0230_M_T0240_M_T0250_M_T0260_pkg
