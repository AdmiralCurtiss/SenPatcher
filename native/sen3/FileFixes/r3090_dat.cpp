#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sha1.h"

namespace SenLib::Sen3::FileFixes::r3090_dat {
std::string_view GetDescription() {
    return "Change name to fixed FMV for second Chapter 3 map travel sequence.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r3090.dat",
            199489,
            SenPatcher::SHA1FromHexString("4e774a1dded6b6054525768a908fa5fd6b03ad25"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        bin[0x4926] = 0x66; // insa_09 -> insa_f9

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r3090_dat
