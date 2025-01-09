#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3400[] = "Terminology fixes in Ordis (Noble District).";
}

namespace SenLib::Sen3::FileFixes::t3400_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3400.dat",
            88561,
            HyoutaUtils::Hash::SHA1FromHexString("d48fd0f978975b3d4854d72475ba670076961db8"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // the port city -> the Port City
        bin[0xcb69] = 0x50;
        bin[0xcb6e] = 0x43;

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3400_dat
