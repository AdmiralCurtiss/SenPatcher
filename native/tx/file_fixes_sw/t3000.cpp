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

namespace SenLib::TX::FileFixesSw::t3000 {
std::string_view GetDescription() {
    return "Text fixes in Shopping Street.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t3000.dat",
            376201,
            HyoutaUtils::Hash::SHA1FromHexString("c6a65c6468bb03cd3f6d404242c8d2ad0f2b37ca"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // the Kagemaru Series 2 list is missing one item and has misspelled another, fix
        {
            static constexpr size_t offset = 0x56d69;
            static constexpr size_t size = 0xc3;
            std::vector<char> tmp;
            tmp.resize(size);
            std::memcpy(tmp.data(), &bin[offset], size);

            // extra line break
            tmp.insert(tmp.begin() + 0x7b, (char)0x01);

            // misspelling
            tmp.insert(tmp.begin() + 0x9d, (char)0x2d);
            tmp[0x9e] = 0x53;

            // missing item
            static constexpr auto pudgy = STR_SPAN("Pudgy Kagemaru\x01");
            tmp.insert(tmp.begin() + 0x99, pudgy.begin(), pudgy.end());

            // apply
            patcher.ReplaceCommand(offset, size, tmp);
        }

        // "Monsters have been rampaging through Shopping Street.."
        // double period
        patcher.RemovePartialCommand(0x20f1c, 0x80, 0x20f53, 0x1);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t3000
