#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sen/sen_script_patcher.h"
#include "../../sha1.h"
#include "../../util/vector.h"

namespace SenLib::Sen3::FileFixes::r4200_dat {
std::string_view GetDescription() {
    return "Terminology fixes in South Ostia Highway.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r4200.dat",
            123489,
            SenPatcher::SHA1FromHexString("d6dcc55f71cf2e6193c6a33b53b8879c0d4d5958"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Hmph, so the Divine Angler of Erebonia has been{n}chosen... I have another rival now.
        // -> Hmph, you may end up as Erebonia's Divine Angler...{n}It seems I have another rival
        // now.
        {
            std::vector<char> ms;
            WriteAtEnd(ms, GetSpan(bin, 0x27ed, 4));
            WriteAtEnd(ms, GetSpan(bin, 0x7e3c, 4));
            WriteAtEnd(ms, GetSpan(bin, 0xc398, 4));
            WriteAtEnd(ms, GetSpan(bin, 0x55d7, 4));
            ms.push_back(0x73);
            WriteAtEnd(ms, GetSpan(bin, 0x80e8, 9));
            WriteAtEnd(ms, GetSpan(bin, 0x501f, 3));
            WriteAtEnd(ms, GetSpan(bin, 0x80d8, 0xd));
            WriteAtEnd(ms, GetSpan(bin, 0x7260, 4));
            ms.push_back(0x49);
            WriteAtEnd(ms, GetSpan(bin, 0xad35, 7));
            patcher.ReplacePartialCommand(0x8075, 0xab, 0x80d1, 0x33, ms);
        }

        // Space/Mirage -> space/mirage
        bin[0x13843] = 0x73;
        bin[0x1384e] = 0x6d;

        // expert angler -> Master Fisher
        WriteAt(bin, 0x1b5ee, {{0x4d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x01, 0x46, 0x69, 0x73, 0x68}});
        WriteAt(bin, 0x1d1f9, {{0x4d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x46, 0x69, 0x73, 0x68}});

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r4200_dat
