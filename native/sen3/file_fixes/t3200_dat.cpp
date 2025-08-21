#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3200[] =
    "Fix name consistency issues in Ordis (North Street).";
}

namespace SenLib::Sen3::FileFixes::t3200_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3200.dat",
            50273,
            HyoutaUtils::Hash::SHA1FromHexString("11f838467868bad61623e965fbc3c0607d2a3356"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);
        using namespace HyoutaUtils::Vector;

        // Brigadier General Wallace -> Brigadier General Bardias
        WriteAt(bin, 0x6057, {{0x42, 0x61, 0x72, 0x64, 0x69, 0x61, 0x73}});

        // General Aurelia -> General Le Guin
        WriteAt(bin, 0x60a9, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});
        WriteAt(bin, 0x9e1b, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});

        // "The night’s really kicking\x01into high gear! I love festivals!"
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with woman walking around)
        patcher.ReplacePartialCommand(0x68a4, 0x95, 0x68b4, 3, {{'\''}});

        // "#E[5]#M_4#B_0Now, I’d like to give you these."
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with Glycine)
        patcher.ReplacePartialCommand(0x6a45, 0x85, 0x6aac, 3, {{'\''}});

        // "#E[5]#M_4#B_0However, since she’s still a child,\x01I need to send her home before
        // it\x01gets too late."
        // "I’ll be sure to send her off with a\x01lovely bouquet for all her efforts."
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with Glycine)
        patcher.ReplacePartialCommand(0x6e0f, 0x164, 0x6f29, 3, {{'\''}});
        patcher.ReplacePartialCommand(0x6e0f, 0x164, 0x6ee2, 3, {{'\''}});

        // "#E_8#M_AI shouuuld get home now that\x01I’m done running errands, but...it\x01won't hurt
        // to take a peek, right?"
        // wrong apostrophe
        // (chapter 3, 6/17 morning, NPC dialogue with Monique)
        patcher.ReplacePartialCommand(0x77a2, 0x97, 0x77f2, 3, {{'\''}});

        // "I shouuuld get home now that\x01I’m done running errands, but...it\x01won't hurt to take
        // a peek, right?"
        // wrong apostrophe
        // (chapter 3, 6/17 morning, NPC dialogue with Monique)
        patcher.ReplacePartialCommand(0x7853, 0x6c, 0x7878, 3, {{'\''}});

        // "Wow, it’s so lively! It feels extra refreshing\x01after we missed out last year."
        // wrong apostrophe
        // (chapter 3, 6/18 festival night, NPC dialogue with Angelo)
        patcher.ReplacePartialCommand(0x7b08, 0xb1, 0x7b16, 3, {{'\''}});

        // "#3KWell, even though he’s retired now,\x01I could tell he must’ve been\x01rather
        // accomplished in his day."
        // wrong apostrophe
        // (chapter 3, 6/17 morning, cutscene after meeting Count Egret)
        patcher.ReplacePartialCommand(0x9767, 0x70, 0x97ab, 3, {{'\''}});
        patcher.ReplacePartialCommand(0x9767, 0x70, 0x9785, 3, {{'\''}});

        // "#4KThat’s very kind of you."
        // wrong apostrophe
        // (chapter 3, 6/17 morning, cutscene after meeting Count Egret)
        patcher.ReplacePartialCommand(0x97f7, 0x26, 0x9805, 3, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3200_dat
