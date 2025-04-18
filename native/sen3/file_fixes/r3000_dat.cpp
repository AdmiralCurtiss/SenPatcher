﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_r3000[] =
    "Fix terminology consistency issues in West Lamare Highway 2.";
}

namespace SenLib::Sen3::FileFixes::r3000_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r3000.dat",
            50865,
            HyoutaUtils::Hash::SHA1FromHexString("9bde3d287daabc7df76207d20b04da2ad1c7120a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // port city -> Port City
        bin[0x40c3] = 0x50;
        bin[0x40c8] = 0x43;

        // The Juno Naval Fortress -> Juno Naval Fortress
        patcher.RemovePartialCommand(0xbd09, 0x94, 0xbd19, 0x4);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r3000_dat
