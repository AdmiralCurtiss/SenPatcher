﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_ada[] =
    "Fix name consistency issues in conversations with Ada.";
}

namespace SenLib::Sen3::FileFixes::tk_ada_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_ada.dat",
            1857,
            HyoutaUtils::Hash::SHA1FromHexString("4c1c3cd56be5eb1a950812709685b2f029096f2d"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Miss Arseid -> Ms. Arseid (esmelas garden party, after talk with priscilla)
        patcher.ReplacePartialCommand(0x2c0, 0xaf, 0x31a, 0x3, {{0x73, 0x2e}});

        // smart quote fix (same as above for first, unsure for second.
        // it should be after the 'From A-da to Frit-Z' sidequest but I have no idea where she ends
        // up standing around there...)
        patcher.ReplacePartialCommand(0x2c0, 0xaf, 0x2da, 0x3, {{0x27}});
        patcher.ReplacePartialCommand(0x4a6, 0x52, 0x4b8, 0x3, {{0x27}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_ada_dat
