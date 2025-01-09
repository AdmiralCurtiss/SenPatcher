#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m5319[] =
    "Text fixes in Witch's Briar Castle - Midpoint 2.";
}

namespace SenLib::TX::FileFixesSw::m5319 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m5319.dat",
            15673,
            HyoutaUtils::Hash::SHA1FromHexString("51da8bb05afd0b8cd1c72cfacfb369534ed7ba91"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);


        // Two instances of the speaker name being left in Japanese -- should just be 'Voice'.
        patcher.ReplacePartialCommand(0x24bf, 0x7, 0x24c0, 0x3, {{0x56, 0x6f, 0x69, 0x63, 0x65}});
        patcher.ReplacePartialCommand(0x24f6, 0x7, 0x24f7, 0x3, {{0x56, 0x6f, 0x69, 0x63, 0x65}});

        // While we're here, improve a linebreak.
        std::swap(bin[0x3502], bin[0x350d]);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m5319
