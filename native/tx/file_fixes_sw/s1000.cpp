#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s1000[] = "Text fixes in Kokonoe Shrine (outside).";
}

namespace SenLib::TX::FileFixesSw::s1000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s1000.dat",
            137145,
            HyoutaUtils::Hash::SHA1FromHexString("5a124a8350ea02590071344aead519d0f1105e7e"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1P...!#10W\x01#1000W#5SWhat is THAT?!"
        // patcher.ReplacePartialCommand(0x13609, 0x2d, 0x13623, 0x11, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s1000
