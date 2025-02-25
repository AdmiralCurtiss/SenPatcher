﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
// the message in insa09 suffices
//__declspec(dllexport) char SenPatcherFix_1_c0000[] =
//    "Change name to fixed FMV for Chapter 2 map travel sequence.";
}

namespace SenLib::Sen3::FileFixes::c0000_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c0000.dat",
            45825,
            HyoutaUtils::Hash::SHA1FromHexString("a559aad2b0919264711604499827c8cca82333f8"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        bin[0x32e3] = 0x66; // insa_05 -> insa_f5

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c0000_dat
