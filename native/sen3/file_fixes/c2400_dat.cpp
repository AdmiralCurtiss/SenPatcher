#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c2400[] =
    "Fix spacing in textbox in Heimdallr (Dreichels Plaza)";
}

namespace SenLib::Sen3::FileFixes::c2400_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c2400.dat",
            193753,
            HyoutaUtils::Hash::SHA1FromHexString("1fab33eb9f5b35fb986da4d52033a9cb7e1e9429"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "            The Lionheart\x01         Dreichels Reise Arnor"
        // This is supposed to be centered in the textbox (see JP equivalent) but instead it's
        // weirdly pushed to the right side. Adjust this so it looks better.
        {
            std::vector<char> tmp;
            tmp.assign(&bin[0x41e5], &bin[0x41e5] + 0x3f);
            tmp.erase(tmp.begin() + 0x1e, tmp.begin() + 0x27);
            tmp.erase(tmp.begin() + 0x4, tmp.begin() + 0x10);
            static constexpr std::array<char, 5> newspace({{'\xE3', '\x80', '\x80', ' ', ' '}});
            tmp.insert(tmp.begin() + 0x4, newspace.begin(), newspace.end());
            patcher.ReplaceCommand(0x41e5, 0x3f, tmp);
        }

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c2400_dat
