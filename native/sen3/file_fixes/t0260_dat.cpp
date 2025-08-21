#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0260[] =
    "Fix reference to Hamilton's gender (School hangar).";
}

namespace SenLib::Sen3::FileFixes::t0260_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0260.dat",
            122593,
            HyoutaUtils::Hash::SHA1FromHexString("f00fc1a818c84469fd34cfb593d03ad424393ace"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        using namespace HyoutaUtils::Vector;

        SenScriptPatcher patcher(bin);

        // switch gender in line from Tita
        {
            std::vector<char> ms;
            auto old = GetSpan(bin, 0x122c7, 0xcb);
            WriteAtEnd(ms, GetSpan(old, 0, 0xf));
            ms.push_back(0x73);
            WriteAtEnd(ms, GetSpan(old, 0xf, 0x75));
            ms.push_back(0x65);
            ms.push_back(0x72);
            WriteAtEnd(ms, GetSpan(old, 0x86, 0xe));
            ms.push_back(0x73);
            WriteAtEnd(ms, GetSpan(old, 0x94, 0x37));
            patcher.ReplaceCommand(0x122c7, 0xcb, ms);
        }

        // switch gender in line from George
        {
            std::vector<char> ms;
            auto old = GetSpan(bin, 0x123cb, 0x92);
            WriteAtEnd(ms, GetSpan(old, 0, 0x33));
            ms.push_back(0x65);
            ms.push_back(0x72);
            WriteAtEnd(ms, GetSpan(old, 0x35, 0x19));
            ms.push_back(0x73);
            WriteAtEnd(ms, GetSpan(old, 0x4e, 0x17));
            ms.push_back(0x65);
            ms.push_back(0x72);
            WriteAtEnd(ms, GetSpan(old, 0x67, 0x2b));
            patcher.ReplaceCommand(0x123cb, 0x92, ms);
        }

        // switch gender in line from Schmidt
        patcher.ExtendPartialCommand(0x1380a, 0x73, 0x1386e, {{0x73}});

        // "#8KAnd she’ll stay plenty dry\x01cooped up in here."
        // wrong apostrophe
        // (chapter 3, 6/10 afternoon, event with Celine/Valimar)
        patcher.ReplacePartialCommand(0x197a4, 0x3b, 0x197b5, 3, {{'\''}});

        // "#E[A]#M_A#B_0I figure this way, I'll be able brag\x01to Uncle Makarov and all my
        // friends."
        // able brag -> able to brag
        // (7/9 morning, school hangar, talk to Mint after starting the Radio Stars sidequest)
        patcher.ExtendPartialCommand(0x16761, 0xa8, 0x167de, STR_SPAN("to "));

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0260_dat
