#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3220[] =
    "Fix name consistency issues in Ordis (Orbal Factory).";
}

namespace SenLib::Sen3::FileFixes::t3220_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3220.dat",
            18977,
            HyoutaUtils::Hash::SHA1FromHexString("4cdac7c910724aa27bfe5ada8fc25f9b9118833b"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);
        using namespace HyoutaUtils::Vector;

        // General Aurelia -> General Le Guin
        WriteAt(bin, 0x141e, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});
        WriteAt(bin, 0x1e43, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});

        // "#K#0TThat’s right! We’re here\x01for your request."
        // wrong apostrophe
        // (chapter 3, 6/17 morning, when starting the Love Me Jaded quest)
        patcher.ReplacePartialCommand(0x2742, 0x3c, 0x2760, 3, {{'\''}});
        patcher.ReplacePartialCommand(0x2742, 0x3c, 0x2752, 3, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3220_dat
