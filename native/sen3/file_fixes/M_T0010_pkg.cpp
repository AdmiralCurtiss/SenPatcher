#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/decompress_helper.h"
#include "sen/file_getter.h"
#include "sen3/file_fixes.h"
#include "sen3/patch_pkg.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_8_M_T0010[] = "Fix dorm room sign for Valerie.";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_sign09.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::M_T0010_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/asset/D3D11/M_T0010.pkg",
            179678858,
            HyoutaUtils::Hash::SHA1FromHexString("e6ac286949f5b408d28b2ee26fae62aad981db2f"));
        if (!file) {
            return false;
        }
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }

        auto& bin = file->Data;
        auto tex =
            PatchSingleFileInPkg(bin.data(), bin.size(), patch->data(), patch->size(), 411, 76);

        result.emplace_back(std::move(tex), file->Filename, SenPatcher::P3ACompressionType::None);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::M_T0010_pkg
