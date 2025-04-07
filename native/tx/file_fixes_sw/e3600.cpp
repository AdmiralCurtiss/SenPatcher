#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_e3600[] = "Text fixes in Gallery.";
}

namespace SenLib::TX::FileFixesSw::e3600 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/e3600.dat",
            19387,
            HyoutaUtils::Hash::SHA1FromHexString("0a9fe60e8a5896123427a7c6ce915b30cebb787a"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);


        // Ryuta -> Ryouta
        patcher.ExtendPartialCommand(0x1818, 0xb, 0x181D, {{'o'}});
        patcher.ExtendPartialCommand(0x1c38, 0xb, 0x1C3D, {{'o'}});

        // "Wish☆Wing" -> "Wish★Wing"
        bin[0xCA3] = '\x85';


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::e3600
