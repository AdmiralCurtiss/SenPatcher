#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/vector.h"

namespace SenLib::Sen3::FileFixes::c3010_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Heimdallr (Cathedral).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c3010.dat",
            154497,
            SenPatcher::SHA1FromHexString("648885e17c3f2371f9a71b07990e054db09242fb"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Sept-Terrion -> Sept-Terrions
        patcher.ExtendPartialCommand(0xed1b, 0x4e, 0xed66, {{0x73}});
        patcher.ExtendPartialCommand(0xf24e, 0x10a, 0xf2e9, {{0x73}});

        // an immortal -> an Immortal
        bin[0x124da] = 0x49;

        // an immortal -> immortal
        patcher.RemovePartialCommand(0x12feb, 0xfd, 0x13015, 0x3);

        // removed 'but...'
        patcher.RemovePartialCommand(0x13147, 0x3d, 0x1317a, 0x7);

        // Sir Lysander! Sir Gaius! -> Father Thomas! Father Gaius!
        std::swap(bin[0x1d992], bin[0x1d99a]);
        patcher.ReplacePartialCommand(
            0x1d958, 0x65, 0x1d967, 0x10, {{0x46, 0x61, 0x74, 0x68, 0x65, 0x72, 0x20,
                                            0x54, 0x68, 0x6f, 0x6d, 0x61, 0x73, 0x21,
                                            0x20, 0x46, 0x61, 0x74, 0x68, 0x65}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c3010_dat
