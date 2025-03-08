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
static constexpr char PatchData[] = {
#include "embed_sen3_0sign003.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::O_C04OBJ01_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
              const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub) {
    try {
        // first try to get this as a pka-referencing pkg
        std::optional<SenPatcher::CheckedFileResult> file = getCheckedFilePkaStub(
            "data/asset/D3D11/O_C04OBJ01.pkg",
            680,
            HyoutaUtils::Hash::SHA1FromHexString("ffe4149578dc62ac6d95a7889adf58ff05966bde"));
        if (!file) {
            // try the full pkg instead, this is less optimal filesize-wise but more compatible
            // since it also works if eg. the user has extracted the .pka and only has the .pkgs
            // lying around for modding purposes
            file = getCheckedFile(
                "data/asset/D3D11/O_C04OBJ01.pkg",
                215278,
                HyoutaUtils::Hash::SHA1FromHexString("33519219ed647067bc8a52a9e2ebc850cb9821fd"));
        }
        if (!file) {
            return false;
        }
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }

        auto& bin = file->Data;
        auto tex = PatchSingleFileInPkg(
            bin.data(), bin.size(), patch->data(), patch->size(), 6, 1, &vanillaPKAs);

        result.emplace_back(std::move(tex),
                            // this fix should not apply when playing in Japanese
                            SenPatcher::InitializeP3AFilename("data/asset/D3D11_us/O_C04OBJ01.pkg"),
                            SenPatcher::P3ACompressionType::None);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::O_C04OBJ01_pkg
