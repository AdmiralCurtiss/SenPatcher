﻿#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sen/sen_script_patcher.h"
#include "../../sha1.h"

namespace SenLib::Sen3::FileFixes::v0050_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Derfflinger Car 6.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/v0050.dat",
            53281,
            SenPatcher::SHA1FromHexString("b1f43611bbf1130ff552405bc50d2f75637e49e1"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // synch -> sync
        patcher.RemovePartialCommand(0xb533, 0x185, 0xb62c, 0x1);

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::v0050_dat