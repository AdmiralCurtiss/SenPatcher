#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "tx/tbl.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_mg04[] = "Text fixes in Blade dialogue.";
}

namespace SenLib::TX::FileFixesSw::t_mg04 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_mg04.tbl",
            13636,
            HyoutaUtils::Hash::SHA1FromHexString("114ed5b845b9ce6460c6b3b4bcdbf13a5b3076ca"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();


        // ellipses -> three dots
        bin[0x1837] = '.';
        bin[0x1838] = '.';
        bin[0x1839] = '.';
        bin[0x2E36] = '.';
        bin[0x2E37] = '.';
        bin[0x2E38] = '.';
        bin[0x2F05] = '.';
        bin[0x2F06] = '.';
        bin[0x2F07] = '.';


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t_mg04
