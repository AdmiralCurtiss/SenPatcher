#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c3210[] = "Terminology fixes in Heimdallr (Racecourse).";
}

namespace SenLib::Sen3::FileFixes::c3210_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c3210.dat",
            221217,
            HyoutaUtils::Hash::SHA1FromHexString("34c651df94044c96f600a24ba99eafa2f04d08d1"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Now, we shall begin the award ceremony on{n}behalf of the Imperial Household Agency.	->
        // Now, we shall present the awards on{n}behalf of City Hall.
        {
            using namespace HyoutaUtils::Vector;
            std::vector<char> newcommand;
            auto command = GetSpan(bin, 0x22d9b, 0xb1);
            WriteAtEnd(newcommand, GetSpan(command, 0, 0x1a));
            WriteAtEnd(newcommand, {{0x70, 0x72, 0x65, 0x73, 0x65, 0x6e, 0x74}});
            WriteAtEnd(newcommand, GetSpan(command, 0x1f, 0xa));
            newcommand.push_back(0x73);
            WriteAtEnd(newcommand, GetSpan(command, 0x32, 0xe));
            WriteAtEnd(newcommand, {{0x43, 0x69, 0x74, 0x79, 0x20, 0x48, 0x61, 0x6c, 0x6c}});
            WriteAtEnd(newcommand, GetSpan(command, 0x5d, 0x54));
            patcher.ReplaceCommand(0x22d9b, 0xb1, newcommand);
        }

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c3210_dat
