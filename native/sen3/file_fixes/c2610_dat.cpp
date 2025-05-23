﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c2610[] =
    "Fix terminology consistency issues in Heimdallr (Foresta Tavern & Inn).";
}

namespace SenLib::Sen3::FileFixes::c2610_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c2610.dat",
            23265,
            HyoutaUtils::Hash::SHA1FromHexString("d89a6f84a665a650f32269931c874bf3d1cb0b1b"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Fisherman, 2nd Class -> 2nd Class Fisherman
        {
            using namespace HyoutaUtils::Vector;
            std::vector<char> ms;
            WriteAtEnd(ms, GetSpan(bin, 0x4c83, 9));
            WriteAtEnd(ms, GetSpan(bin, 0x4c77, 10));
            patcher.ReplacePartialCommand(0x4bf2, 0xe2, 0x4c78, 0x14, ms);
        }

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c2610_dat
