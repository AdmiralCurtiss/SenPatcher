#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3410[] = "Fix typography issues in Ordis (church)";
}

namespace SenLib::Sen3::FileFixes::t3410_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3410.dat",
            15396,
            HyoutaUtils::Hash::SHA1FromHexString("5fb19887d7b85c52ac67831a9986bb37035409c4"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);


        // "#E[1]I’ll get started on the decorations\x01for the Summer Festival."
        // wrong apostrophe
        // (chapter 3, 6/17, NPC dialogue with Sienna and Robbins before leaving Ordis)
        patcher.ReplacePartialCommand(0x225b, 0x8d, 0x22a9, 3, {{'\''}});

        // "I-I’ve got it covered."
        // wrong apostrophe
        // (chapter 3, 6/17, NPC dialogue with Sienna and Robbins before leaving Ordis)
        patcher.ReplacePartialCommand(0x26de, 0x21, 0x26e8, 3, {{'\''}});

        // "#E_4#M_A#B_0It's a splendid night, isn’t it?"
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with Marquis Reagan)
        patcher.ReplacePartialCommand(0x2ade, 0x63, 0x2b37, 3, {{'\''}});

        // "We can’t miss tonight's Mass."
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with woman near entrance)
        patcher.ReplacePartialCommand(0x38a8, 0x79, 0x38b5, 3, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3410_dat
