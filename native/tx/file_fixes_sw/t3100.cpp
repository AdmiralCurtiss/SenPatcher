#include <cstring>
#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3100[] = "Text fixes in Kuguraya.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t3100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t3100.dat",
            110673,
            HyoutaUtils::Hash::SHA1FromHexString("1495f4d5f96c84762cc02aa59f7c18ecf995351d"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "I ain't exactly pining fer him if that's\x01what yer insinuat─#1000WOops, I mean..."
        // Chapter 2 Side story (EV_11_02_01)
        // harmless
        // patcher.ReplacePartialCommand(0xf84e, 0x5c, 0xf893, 0x15, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t3100
