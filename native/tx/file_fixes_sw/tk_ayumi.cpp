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
__declspec(dllexport) char SenPatcherFix_1_ztk_ayumi[] = "Text fixes in conversations with Ayumi.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::tk_ayumi {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_ayumi.dat",
            14041,
            HyoutaUtils::Hash::SHA1FromHexString("93cd1b5267f3633374150bc298ac7290f9568159"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // This file has received a very confusing Switch v1.0.1 update that basically just made
        // everything worse.
        // - All the "Obtained [item] x3" messages were changed to remove the space between the item
        //   name and the 'x3'.
        // - They added capitalization to Karate Team and Cooking Club, even though they made them
        //   lowercase *everywhere else*.
        // So we're going to revert most of those changes.

        for (int offset :
             {0x32B,  0x333,  0x53B,  0x542,  0x9B9,  0x9C1,  0xCA0,  0xCA8,  0x177E, 0x1785,
              0x1D0A, 0x1D12, 0x2884, 0x288B, 0x2DE1, 0x2DE9, 0x2DF5, 0x2DFC, 0x2ED0, 0x2ED8,
              0x2F07, 0x2F0E, 0x303F, 0x3047, 0x309A, 0x30A1, 0x31D5, 0x31DC, 0x3431, 0x3439}) {
            char& c = bin[static_cast<size_t>(offset)];
            if (c >= 'A' && c <= 'Z') {
                c = (c + ('a' - 'A'));
            }
        }

        // Obtained Homemade Pumpkin Pie x3.
        // After Story, 10/30, School Annex 1F
        patcher.ExtendPartialCommand(0x148, 0x17, 0x159, {{' '}});
        // Obtained Homemade Apple Pie x3.
        // Chapter 7, 7/8, before searching Gorou's apartment, School Annex 1F (if Sora is in party)
        patcher.ExtendPartialCommand(0xedf, 0x17, 0xef0, {{' '}});
        // Obtained Homemade Apple Pie x3.
        // Chapter 7, 7/8, before searching Gorou's apartment, School Annex 1F (otherwise)
        patcher.ExtendPartialCommand(0xfa2, 0x17, 0xfb3, {{' '}});
        // Obtained Mushroom Sandwich x2.
        // Chapter 5, 6/4, Clubhouse
        patcher.ExtendPartialCommand(0x1b93, 0x17, 0x1ba4, {{' '}});
        // Obtained Homemade Madeleine x4.
        // Chapter 3, 5/9, near the end of the chapter, Nanahoshi Mall 1F
        patcher.ExtendPartialCommand(0x29a3, 0x17, 0x29b4, {{' '}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_ayumi
