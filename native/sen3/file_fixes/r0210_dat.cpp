﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_r0210[] =
    "Fix name consistency issues in North Sutherland Highway 2.";
}

namespace SenLib::Sen3::FileFixes::r0210_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r0210.dat",
            32097,
            HyoutaUtils::Hash::SHA1FromHexString("b636a0c274714e93b74db3871e7019bf6ca39a7c"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        using namespace HyoutaUtils::Vector;

        // General Aurelia -> General Le Guin
        WriteAt(bin, 0x39a9, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});
        WriteAt(bin, 0x3e9b, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});

        // Brigadier General Wallace -> Brigadier General Bardias
        WriteAt(bin, 0x3eb9, {{0x42, 0x61, 0x72, 0x64, 0x69, 0x61, 0x73}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r0210_dat
