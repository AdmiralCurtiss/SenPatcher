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
__declspec(dllexport) char SenPatcherFix_8_O_V80TNK00[] = "Fix RMP insignia texture.";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_5vCrest01.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_V80TNK00_pkg {
static bool TryApplyInternal(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
                             std::vector<SenPatcher::P3APackFile>& result,
                             SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                             const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub,
                             std::string_view inpath,
                             std::string_view outpath,
                             size_t stubsize,
                             const HyoutaUtils::Hash::SHA1& stubhash,
                             size_t realsize,
                             const HyoutaUtils::Hash::SHA1& realhash,
                             const char* patchData,
                             size_t patchLength,
                             size_t expectedFileCount,
                             size_t fileToPatch) {
    try {
        // first try to get this as a pka-referencing pkg
        std::optional<SenPatcher::CheckedFileResult> file =
            getCheckedFilePkaStub(inpath, stubsize, stubhash);
        if (!file) {
            // try the full pkg instead, this is less optimal filesize-wise but more compatible
            // since it also works if eg. the user has extracted the .pka and only has the .pkgs
            // lying around for modding purposes
            file = getCheckedFile(inpath, realsize, realhash);
        }
        if (!file) {
            return false;
        }
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }

        auto& bin = file->Data;
        auto tex = PatchSingleFileInPkg(bin.data(),
                                        bin.size(),
                                        patchData,
                                        patchLength,
                                        expectedFileCount,
                                        fileToPatch,
                                        &vanillaPKAs);

        std::array<char, 0x100> p3aFileName;
        if (!SenPatcher::CopyToP3AFilename(p3aFileName, outpath)) {
            return false;
        }
        result.emplace_back(std::move(tex), p3aFileName, SenPatcher::P3ACompressionType::None);

        return true;
    } catch (...) {
        return false;
    }
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }

        if (!TryApplyInternal(
                getCheckedFile,
                result,
                vanillaPKAs,
                getCheckedFilePkaStub,
                "data/asset/D3D11/O_V80TNK00.pkg",
                "data/asset/D3D11_us/O_V80TNK00.pkg",
                1576,
                HyoutaUtils::Hash::SHA1FromHexString("f34658a1e4ab97bd0c80758a2e8d6d8115f63f7e"),
                13936345,
                HyoutaUtils::Hash::SHA1FromHexString("c11948b2b6b3d5bd746a75642c9bea167c6c7dc3"),
                patch->data(),
                patch->size(),
                14,
                1)) {
            return false;
        }
        if (!TryApplyInternal(
                getCheckedFile,
                result,
                vanillaPKAs,
                getCheckedFilePkaStub,
                "data/asset/D3D11/O_V81TNK00.pkg",
                "data/asset/D3D11_us/O_V81TNK00.pkg",
                1800,
                HyoutaUtils::Hash::SHA1FromHexString("f770b80b502759f22a06285813ac82e725918aab"),
                12740193,
                HyoutaUtils::Hash::SHA1FromHexString("4640705fad652ec00e0909de5f5efd321037c35b"),
                patch->data(),
                patch->size(),
                16,
                1)) {
            return false;
        }
        if (!TryApplyInternal(
                getCheckedFile,
                result,
                vanillaPKAs,
                getCheckedFilePkaStub,
                "data/asset/D3D11/O_V81TNK01.pkg",
                "data/asset/D3D11_us/O_V81TNK01.pkg",
                1464,
                HyoutaUtils::Hash::SHA1FromHexString("da76ef142c2ef110cdb7cbe7af71f2ca7b8f5674"),
                13081197,
                HyoutaUtils::Hash::SHA1FromHexString("6a6a300aa27f1e3e1de83bd10f85a9f4da8b47b5"),
                patch->data(),
                patch->size(),
                13,
                1)) {
            return false;
        }
        if (!TryApplyInternal(
                getCheckedFile,
                result,
                vanillaPKAs,
                getCheckedFilePkaStub,
                "data/asset/D3D11/O_V81TNK02.pkg",
                "data/asset/D3D11_us/O_V81TNK02.pkg",
                1576,
                HyoutaUtils::Hash::SHA1FromHexString("60c6edd448693583efa474511726df987be66269"),
                13627109,
                HyoutaUtils::Hash::SHA1FromHexString("4200cbdb598e8b93cf3c5d2e134a9a730449f45c"),
                patch->data(),
                patch->size(),
                14,
                1)) {
            return false;
        }
        if (!TryApplyInternal(
                getCheckedFile,
                result,
                vanillaPKAs,
                getCheckedFilePkaStub,
                "data/asset/D3D11/O_V81TNK03.pkg",
                "data/asset/D3D11_us/O_V81TNK03.pkg",
                1464,
                HyoutaUtils::Hash::SHA1FromHexString("5901f1163bf89a5237b0cfb5fe3f11746c7e8874"),
                14015758,
                HyoutaUtils::Hash::SHA1FromHexString("29383ab49911b20d973c3a8820d18200a795aa1b"),
                patch->data(),
                patch->size(),
                13,
                1)) {
            return false;
        }
        if (!TryApplyInternal(
                getCheckedFile,
                result,
                vanillaPKAs,
                getCheckedFilePkaStub,
                "data/asset/D3D11/O_V82TNK00.pkg",
                "data/asset/D3D11_us/O_V82TNK00.pkg",
                2136,
                HyoutaUtils::Hash::SHA1FromHexString("7c6df7702330ec465a05fc2fa44449ef89870391"),
                8118795,
                HyoutaUtils::Hash::SHA1FromHexString("c99da45b6681d5da1848f25f1943e836e320e819"),
                patch->data(),
                patch->size(),
                19,
                2)) {
            return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::O_V80TNK00_pkg
