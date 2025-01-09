#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c2430[] =
    "Terminology fixes in Valflame Palace (Esmelas Garden).";
}

namespace SenLib::Sen3::FileFixes::c2430_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c2430.dat",
            254401,
            HyoutaUtils::Hash::SHA1FromHexString("2fae040a29bafe3cf202c2d5eddadef2424523d8"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Imperial Household Agency -> City Hall
        std::swap(bin[0x10560],
                  bin[0x1056c]); // switch line break position in following replace line
        patcher.ReplacePartialCommand(
            0x103e7,
            0x1ab,
            0x1053e,
            0x1d,
            {{0x43, 0x69, 0x74, 0x79, 0x20, 0x48, 0x61, 0x6c, 0x6c}}); // eugent
        patcher.ReplacePartialCommand(
            0x1fcbc,
            0xe2,
            0x1fcf6,
            0x1d,
            {{0x43, 0x69, 0x74, 0x79, 0x20, 0x48, 0x61, 0x6c, 0x6c}}); // rufus

        // Highness -> Majesty
        patcher.ReplacePartialCommand(
            0xa524,
            0x68,
            0xa542,
            0x8,
            {{0x4d, 0x61, 0x6a, 0x65, 0x73, 0x74, 0x79}}); // guard to eugent

        // royal family -> Imperial family
        patcher.ReplacePartialCommand(
            0xa155, 0x135, 0xa1aa, 0x3, {{0x49, 0x6d, 0x70, 0x65, 0x72, 0x69}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c2430_dat
