﻿#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sha1.h"

namespace SenLib::Sen3::FileFixes::t0100_dat {
std::string_view GetDescription() {
    return "Textbox overflow in Recette Bakery.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0100.dat",
            56609,
            SenPatcher::SHA1FromHexString("02b743e33e402065f67af2fa318064f41314d353"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // chapter 1 system message for going to the einhel keep
        // swap linebreak forwards so line doesn't overflow textbox
        std::swap(bin[0x8a0a], bin[0x8a19]);

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0100_dat