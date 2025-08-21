#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c0240[] =
    "Fix typography in Vingt-Sept Cafe (Crossbell)";
}

namespace SenLib::Sen3::FileFixes::c0240_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c0240.dat",
            15505,
            HyoutaUtils::Hash::SHA1FromHexString("624bacffca563287dd2aee6ee443c172e4f68455"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E[5]I’m really enjoying myself, too. ㈱"
        // wrong apostrophe
        // (chapter 2, 5/21 after Olivier joins)
        patcher.ReplacePartialCommand(0x37d6, 0x82, 0x3831, 3, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c0240_dat
