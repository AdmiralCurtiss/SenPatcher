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
__declspec(dllexport) char SenPatcherFix_8_I_JMP009[] =
    "Fix name of Eisengard Mountain Range on world map.";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_jump009.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::I_JMP009_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/asset/D3D11_us/I_JMP009.pkg",
            6144541,
            HyoutaUtils::Hash::SHA1FromHexString("05f162efc3d880e94398ae4e10108aa018f30fee"));
        if (!file) {
            return false;
        }
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }

        auto& bin = file->Data;
        auto tex = PatchSingleFileInPkg(bin.data(), bin.size(), patch->data(), patch->size(), 2, 1);

        result.emplace_back(std::move(tex), file->Filename, SenPatcher::P3ACompressionType::None);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::I_JMP009_pkg
