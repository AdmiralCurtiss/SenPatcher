#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_place[] = "Fix minor error in area names.";
}

namespace SenLib::Sen3::FileFixes::t_place {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_place.tbl",
            30026,
            HyoutaUtils::Hash::SHA1FromHexString("6f74b48ecea8b47ec2d951d0cdc29f313ec82738"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        std::swap(bin[0x11c1], bin[0x11c1 + 1]);
        std::swap(bin[0x3a1f], bin[0x3a1f + 1]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t_place
