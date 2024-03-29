#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"

namespace SenLib::Sen3::FileFixes::c0830_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Crossbell (Heiyue).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c0830.dat",
            30265,
            SenPatcher::SHA1FromHexString("bb6a5b0a31ebe05ceb06ae8c0b773e2f0b8823af"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // Heiyue Trading Company -> Heiyue trading company
        bin[0x19bc] = 0x74;
        bin[0x19c4] = 0x63;
        bin[0x685e] = 0x74;
        bin[0x6866] = 0x63;

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c0830_dat
