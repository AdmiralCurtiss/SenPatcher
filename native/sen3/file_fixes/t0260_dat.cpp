#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/vector.h"

namespace SenLib::Sen3::FileFixes::t0260_dat {
std::string_view GetDescription() {
    return "Fix Hamilton gender (scene 1, text only).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0260.dat",
            122593,
            SenPatcher::SHA1FromHexString("f00fc1a818c84469fd34cfb593d03ad424393ace"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
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

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0260_dat
