﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c0200[] =
    "Fix name consistency issues in Crossbell (Central Square).";
}

namespace SenLib::Sen3::FileFixes::c0200_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c0200.dat",
            97081,
            HyoutaUtils::Hash::SHA1FromHexString("8a6e007a75ff9d1b06cc96f67d383ff7de6d2eb6"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // Business Owner's Association -> Business Owners' Association
        std::swap(bin[0x14a7b], bin[0x14a7c]);
        std::swap(bin[0x14c7b], bin[0x14c7c]);
        std::swap(bin[0x14d34], bin[0x14d35]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c0200_dat
