﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m3420[] =
    "Fix name consistency issues in Dark Dragon's Nest (Entrance).";
}

namespace SenLib::Sen3::FileFixes::m3420_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/m3420.dat",
            20441,
            HyoutaUtils::Hash::SHA1FromHexString("2eff0eb2d6a770b42069380f42d7daecdd33a96d"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // the Naval Fortress -> the naval fortress
        bin[0x2759] = 0x6e;
        bin[0x275f] = 0x66;

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::m3420_dat
