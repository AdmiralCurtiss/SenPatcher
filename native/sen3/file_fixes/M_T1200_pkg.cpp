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
__declspec(dllexport) char SenPatcherFix_8_M_T1200[] = "Fix Chamber's Orbal Factory sign texture.";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_7t12hou23.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
static constexpr char PatchNormalData[] = {
#include "embed_sen3_7t12hou23n.h"
};
static constexpr size_t PatchNormalLength = sizeof(PatchNormalData);
} // namespace

namespace SenLib::Sen3::FileFixes::M_T1200_pkg {
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
                             const char* patchNormalData,
                             size_t patchNormalLength,
                             size_t expectedFileCount,
                             size_t fileToPatch,
                             size_t fileToPatchNormal) {
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
        auto tex2 = PatchSingleFileInPkg(tex.data(),
                                         tex.size(),
                                         patchNormalData,
                                         patchNormalLength,
                                         expectedFileCount,
                                         fileToPatchNormal,
                                         &vanillaPKAs);

        std::array<char, 0x100> p3aFileName;
        if (!SenPatcher::CopyToP3AFilename(p3aFileName, outpath)) {
            return false;
        }
        result.emplace_back(std::move(tex2), p3aFileName, SenPatcher::P3ACompressionType::None);

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
        auto patchNormal = DecompressFromBuffer(PatchNormalData, PatchNormalLength);
        if (!patchNormal) {
            return false;
        }

        if (!TryApplyInternal(
                getCheckedFile,
                result,
                vanillaPKAs,
                getCheckedFilePkaStub,
                "data/asset/D3D11/M_T1200.pkg",
                "data/asset/D3D11_us/M_T1200.pkg",
                8968,
                HyoutaUtils::Hash::SHA1FromHexString("39109f0a086195c3447421b4e3830ca859d52fe0"),
                83433757,
                HyoutaUtils::Hash::SHA1FromHexString("d33eb48e6fcf0774134c1f226d62f4eb46c91e83"),
                patch->data(),
                patch->size(),
                patchNormal->data(),
                patchNormal->size(),
                80,
                28,
                29)) {
            return false;
        }
        if (!TryApplyInternal(
                getCheckedFile,
                result,
                vanillaPKAs,
                getCheckedFilePkaStub,
                "data/asset/D3D11/M_T1220.pkg",
                "data/asset/D3D11_us/M_T1220.pkg",
                17480,
                HyoutaUtils::Hash::SHA1FromHexString("b4fa071b7cc215047671e7ea087ff1e8e32d20c6"),
                89991513,
                HyoutaUtils::Hash::SHA1FromHexString("0e7467719689168886cd3be80be5dda337d118e0"),
                patch->data(),
                patch->size(),
                patchNormal->data(),
                patchNormal->size(),
                156,
                59,
                60)) {
            return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_T1200_pkg
