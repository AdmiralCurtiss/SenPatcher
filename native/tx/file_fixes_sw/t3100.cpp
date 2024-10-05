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

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t3100 {
std::string_view GetDescription() {
    return "Text fixes in Kuguraya.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t3100.dat",
            110721,
            HyoutaUtils::Hash::SHA1FromHexString("63fcba615c3111ce302665f97595eeb0841c3bfe"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "...#14WWhat is going on\x01between you and Kou, Asuka?"
        // that #14W is a command to reduce the speed at which the text is printed, it's a leftover
        // from the JP script which only sets it for one character and then back to the standard
        // speed in order to add a delay mid-dialogue to, probably, match the voice acting.
        // remove it. also move the linebreak.
        std::swap(bin[0xeda1], bin[0xeda9]);
        patcher.RemovePartialCommand(0xed82, 0x3d, 0xed8d, 0x4);

        // "I ain't exactly pining fer him if that's\x01what yer insinuat-#1000WOops, I mean..."
        // patcher.ReplacePartialCommand(0xf86a, 0x5c, 0xf8af, 0x15, "");

        // "protecting themselves from any sort of arm" -> "[...] any sort of harm"
        patcher.ExtendPartialCommand(0xa4c9, 0x122, 0xa5e5, {{0x68}});

        // "But I DID promise my parents that I'd focus on my schoolwork, too.."
        // extra period
        patcher.RemovePartialCommand(0x4379, 0xd2, 0x43fd, 0x1);

        // "By the way, it seems like you've changed a bit lately, Asuka. Not like you've grown soft
        // or anything like that, but kind of like...you've just matured."
        // NPC dialogue from Mayu... unfortunately, she says this when your party is just Kou,
        // without Asuka, so this should not be in second person.
        {
            static constexpr size_t offset = 0x4a69;
            static constexpr size_t size = 0xd2;
            std::vector<char> tmp;
            tmp.resize(size);
            std::memcpy(tmp.data(), &bin[offset], size);

            tmp.erase(tmp.begin() + 0x49, tmp.begin() + 0xa2);
            tmp.erase(tmp.begin() + 0x5, tmp.begin() + 0x42);

            static constexpr auto line1 = STR_SPAN(
                "It seems like Asuka has changed\x01"
                "a bit lately.");
            static constexpr auto line2 = STR_SPAN(
                "It's not that she's grown soft or\x01"
                "anything like that, but more like...\x01"
                "she's just matured.");

            tmp.insert(tmp.begin() + 0xc, line2.begin(), line2.end());
            tmp.insert(tmp.begin() + 0x5, line1.begin(), line1.end());

            patcher.ReplaceCommand(offset, size, tmp);
        }

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t3100
