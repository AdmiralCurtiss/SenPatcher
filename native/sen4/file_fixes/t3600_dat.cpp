#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3600[] = "Typo in Casper's name.";
}

namespace SenLib::Sen4::FileFixes::t3600_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3600.dat",
            93585,
            HyoutaUtils::Hash::SHA1FromHexString("21ac0c6c99dcf57c0f75b00d44d42d8dae99fe3f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // Caspar -> Casper
        bin[0x41e8] = 0x65;
        bin[0x4289] = 0x65;
        bin[0x4502] = 0x65;
        bin[0x45a3] = 0x65;

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen4::FileFixes::t3600_dat
