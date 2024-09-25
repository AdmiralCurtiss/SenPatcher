#include <array>
#include <cstring>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::system_dat {
std::string_view GetDescription() {
    return "Text fixes in system messages.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/system.dat",
            206169,
            HyoutaUtils::Hash::SHA1FromHexString("60a77d828e302a734e9af8e45fed05ab65e440c6"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // Revert changed button prompt for voicemail tutorial.
        // Switch version has different default button mappings.
        bin[0x1d9b7] = 0x34;

        // Reward from Sousuke is listed as 1000 yen but actually gives you 10k.
        patcher.ExtendPartialCommand(0x20855, 0x23, 0x20869, {{0x30}});

        // "Glad I bought picked it up." -> "Glad I picked it up."
        // for MP3 player gift item
        patcher.RemovePartialCommand(0xce0f, 0x64, 0xce5d, 0x7);

        // "Change Element" -> "Exchange Element"
        // for the master relic at the end of the final dungeons
        patcher.ReplacePartialCommand(0x1c692, 0x14, 0x1c695, 0x1, {{0x45, 0x78, 0x63}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::system_dat
