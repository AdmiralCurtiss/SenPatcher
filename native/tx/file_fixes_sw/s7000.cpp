#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s7000[] = "Text fixes in hot springs entrance.";
}

namespace SenLib::TX::FileFixesSw::s7000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s7000.dat",
            46913,
            HyoutaUtils::Hash::SHA1FromHexString("22d5ca04a5aee7cb39d7f8f9e96ee2540a46d264"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "##2P#FYup! He'll probably be back at\x01
        // school tomorrow, teaching like\x01
        // nothing happened!"
        // the double ## at the start breaks the tag parsing, remove one of them
        patcher.RemovePartialCommand(0x3276, 0x5f, 0x327F, 1);

        // "Due to your bonds having grown stronger, your max\x01"
        // "Strike Points have increased to 2, and your max EX Points have increased to 3."
        // wide text box with second line overhang, move linebreak to make it better
        // patcher.ReplacePartialCommand(0xa4b2, 0x10d, 0xa4b5, 0x81, "");
        std::swap(bin[0xA4E7], bin[0xA4F5]);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s7000
