﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0410[] =
    "Fix terminology consistency issues in Einhel Keep Entrance.";
}

namespace SenLib::Sen3::FileFixes::t0410_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0410.dat",
            135089,
            HyoutaUtils::Hash::SHA1FromHexString("4d0f1d1d3b57eb70de562a52f4367495426d1896"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // synched -> synced
        patcher.RemovePartialCommand(0x6c9d, 0x5b, 0x6cdc, 0x1);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0410_dat
