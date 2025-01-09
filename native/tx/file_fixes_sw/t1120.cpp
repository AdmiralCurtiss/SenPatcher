#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1120[] = "Text fixes in school 2F.";
}

namespace SenLib::TX::FileFixesSw::t1120 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1120.dat",
            405329,
            HyoutaUtils::Hash::SHA1FromHexString("053c0071dd3283828989191bcea1265bc2bc988c"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#8WWhen's Yukino gonna get in touch with me#1000W...?"
        // remove leftover slow text print command, there's no natural break in the english version
        // of the line to keep it
        patcher.RemovePartialCommand(0x4c838, 0x42, 0x4c843, 0x3);

        // linebreaks
        std::swap(bin[0x4a215], bin[0x4a21c]);
        std::swap(bin[0x4c12c], bin[0x4c129]);

        // "#1P#800W(She totally did that on purpose!)\x01#1000W#5SWhat a pain in the..."
        // patcher.ReplacePartialCommand(0x3593d, 0x53, 0x35976, 0x18, "");

        // "#800W*click* *click* *click*\x01#9W*sigh* #1000WNo luck."
        // patcher.ReplacePartialCommand(0x4e6dc, 0x3f, 0x4e70b, 0xe, "");

        // "#2P#800WWork, spending time to friends, going\x01to class. #12W...#1000WMaybe even
        // getting my ass\x01handed to me in games."
        // patcher.ReplacePartialCommand(0x4f22b, 0x7f, 0x4f272, 0x36, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1120
