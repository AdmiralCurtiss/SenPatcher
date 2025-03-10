#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0070[] = "Fix minor text issues in Auditorium.";
}

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0070_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0070.dat",
            122570,
            HyoutaUtils::Hash::SHA1FromHexString("eab3c1e32b2287639eab4b87c8ce67a41e0c4d80"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // remove misplaced voice clip (NPC that says the same line in a cutscene where it's voiced)
        // (Final Chapter, very last free movement sequence)
        patcher.RemovePartialCommand(0x4718, 0x31, 0x471b, 0x5);

        // flip text (but not voice) of Alisa's reactions to Angelica (from PS4) (before concert)
        // this change is pretty arguable so I don't think I'll actually use it...
        // patcher.ReplacePartialCommand(0x12ba6, 0x1e, 0x12bb4, 0x3, {{0x2d}});
        // patcher.ReplacePartialCommand(0x1374b, 0x16, 0x13754, 0x1, {{0x68, 0x21, 0x20}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0070_dat
