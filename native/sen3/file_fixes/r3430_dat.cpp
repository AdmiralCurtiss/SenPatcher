#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

namespace SenLib::Sen3::FileFixes::r3430_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Languedoc Canyon - North.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r3430.dat",
            161769,
            SenPatcher::SHA1FromHexString("3efbd8764d61274a8750342972e75143131d7721"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // A-rank bracer -> A-rank level bracer
        {
            std::vector<char> ms;
            std::swap(bin[0x17c53], bin[0x17c60]);
            auto data = GetSpan(bin, 0x17c2f, 0x54);
            WriteAtEnd(ms, GetSpan(data, 0, 0x3c));
            WriteAtEnd(ms, {{0x6c, 0x65, 0x76, 0x65, 0x6c}});
            WriteAtEnd(ms, GetSpan(data, 0x3b, 0x16));
            WriteAtEnd(ms, GetSpan(data, 0x52, 2));
            patcher.ReplaceCommand(0x17c2f, 0x54, ms);
        }

        // Naval Fortress -> naval fortress
        bin[0x23d2c] = 0x6e;
        bin[0x23d32] = 0x66;

        // remove space before !
        patcher.RemovePartialCommand(0xff94, 0x29, 0xffb9, 0x1);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r3430_dat
