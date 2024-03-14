#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"

namespace SenLib::Sen3::FileFixes::v0030_dat {
std::string_view GetDescription() {
    return "Change name to fixed FMV for first Chapter 3 map travel sequence.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/v0030.dat",
            114577,
            SenPatcher::SHA1FromHexString("c44bfadad573ac7cef14739ca2f697c6d06bb156"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        bin[0x11151] = 0x66; // insa_08 -> insa_f8

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::v0030_dat
