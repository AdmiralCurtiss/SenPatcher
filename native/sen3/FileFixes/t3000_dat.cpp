﻿#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"
#include "util/vector.h"

namespace SenLib::Sen3::FileFixes::t3000_dat {
std::string_view GetDescription() {
    return "Aurelia name fixes in Ordis (Business District).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3000.dat",
            86865,
            SenPatcher::SHA1FromHexString("d0fd8367295246ab1eafede393303d34f7f5bcc5"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // General Aurelia -> General Le Guin
        WriteAt(bin, 0x72ad, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});
        WriteAt(bin, 0x7404, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3000_dat
