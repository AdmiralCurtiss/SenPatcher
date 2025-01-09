#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_jump[] = "Fix minor error in fast-travel menu text.";
}

namespace SenLib::Sen3::FileFixes::t_jump {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_jump.tbl",
            22928,
            HyoutaUtils::Hash::SHA1FromHexString("57dd3ac63f52d21b677d5e1e7c20a7f255d0689f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        std::swap(bin[0x1b7d], bin[0x1b7d + 1]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t_jump
