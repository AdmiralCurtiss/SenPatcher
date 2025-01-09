#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s7130[] = "Text fixes in hot springs outdoor bath.";
}

namespace SenLib::TX::FileFixesSw::s7130 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s7130.dat",
            60353,
            HyoutaUtils::Hash::SHA1FromHexString("80e1d0eaa80d62342c9b466b6269cf0d7dba06a5"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "You've always been a hardworker,"
        // missing space
        patcher.ExtendPartialCommand(0x91a9, 0xb4, 0x9207, {{0x20}});

        // "#3C#0T#3S#3CDamn it, I can't reach#15W...\x01#1000W#5SWHOA!"
        // patcher.ReplacePartialCommand(0x447d, 0x43, 0x44b5, 0x8, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s7130
