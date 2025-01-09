#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t2400[] = "Text fixes in Sunshine Road.";
}

namespace SenLib::TX::FileFixesSw::t2400 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t2400.dat",
            373225,
            HyoutaUtils::Hash::SHA1FromHexString("6c5b8b9790061cc7d3d5b9dbd09d0d15a4ad77e3"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#5S#0THaruna"
        // memories menu of the After Story concert has this instead of the correct line, no idea
        // what happened here... patcher.ReplacePartialCommand(0x4d6d5, 0x11, 0x4d6db, 0x9, "");

        // "#2PThe hell? You just got here.\x01...Anyway, are we late?\x01Sorry 'bout that."
        // patcher.ReplacePartialCommand(0x17f7f, 0x53, 0x17f87, 0x49, "");

        // "#3C#500W#0T#3C...But...I just...#14W#250Wdon't...understand..."
        // patcher.ReplacePartialCommand(0x29090, 0xa7, 0x290bc, 0x1a, "");

        // "#3C#500W#1P#3CTen years ago...#16W Wh-#250WWhy...#22W #250WWhy#16W...#250Wwas I
        // born...?"
        // patcher.ReplacePartialCommand(0x29090, 0xa7, 0x29122, 0x13, "");

        // "#1P#6C#6CI was waiting for you."
        // patcher.ReplacePartialCommand(0x34946, 0x29, 0x34954, 0x19, "");

        // "#3C#500W#0T#3C...But...I just...#14W#250Wdon't...understand..."
        // patcher.ReplacePartialCommand(0x37818, 0xa7, 0x37844, 0x1a, "");

        // "#3C#500W#1P#3CTen years ago...#16W Wh-#250WWhy...#22W #250WWhy#16W...#250Wwas I
        // born...?"
        // patcher.ReplacePartialCommand(0x37818, 0xa7, 0x378aa, 0x13, "");

        // "Kou, Hiiragi, and everyone else,\x01too. Forever and always, I'll be\x01here for you."
        // patcher.ReplacePartialCommand(0x3fca3, 0x59, 0x3fca6, 0x54, "");

        // "#E_J#M_9#1PWe've got Sora, Yuuki, Shio, Mitsuki,\x01Rion, Mr. Gorou, Towa...and even
        // Jun\x01and Ryouta on our side."
        // patcher.ReplacePartialCommand(0x40002, 0x77, 0x40012, 0x65, "");

        // "#K#0T#FGod damn it...\x01#1000W#5SGot it! I'll be right there!"
        // patcher.ReplacePartialCommand(0x41821, 0x45, 0x41845, 0x1f, "");

        // "#5S#1PAll right, Morimiya!\x01#1000WLet's make this a night to remember!"
        // patcher.ReplacePartialCommand(0x4469a, 0x50, 0x446bd, 0x2a, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t2400
