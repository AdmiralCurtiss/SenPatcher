#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_r3200[] = "Terminology fixes in Auros Coastal Road.";
}

namespace SenLib::Sen3::FileFixes::r3200_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r3200.dat",
            92145,
            HyoutaUtils::Hash::SHA1FromHexString("48c59f32ff001518dcef78c47b5f2050f3e0de55"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // port city -> Port City
        bin[0x2db1] = 0x50;
        bin[0x2db6] = 0x43;
        bin[0x4bac] = 0x50;
        bin[0x4bb1] = 0x43;

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r3200_dat
