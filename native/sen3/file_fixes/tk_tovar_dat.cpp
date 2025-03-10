﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_toval[] =
    "Fix typography consistency issues in conversations with Toval.";
}

namespace SenLib::Sen3::FileFixes::tk_tovar_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_tovar.dat",
            536,
            HyoutaUtils::Hash::SHA1FromHexString("5f392e50537f7ef779cb276c8d04c4eb7b7198d4"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // smart quote fix (chapter 4 festival, horse race track lounge)
        patcher.ReplacePartialCommand(0x182, 0x33, 0x197, 0x3, {{0x27}});
        patcher.ReplacePartialCommand(0x182, 0x33, 0x191, 0x3, {{0x27}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_tovar_dat
