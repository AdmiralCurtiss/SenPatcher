#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "tx/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_item[] = "Text fixes in item names and descriptions.";
}

namespace SenLib::TX::FileFixesSw::t_item {
static std::string AdjustNewlinesToTwoSpaces(std::string desc) {
    size_t idx = 0;
    std::string s = std::move(desc);
    while (true) {
        size_t nidx = s.find_first_of('\n', idx);
        if (nidx == std::string::npos) {
            break;
        }

        size_t spaces = 0;
        size_t i = nidx + 1;
        while (i < s.size()) {
            if (s[i] == ' ') {
                ++i;
                ++spaces;
            } else {
                break;
            }
        }

        if (spaces != 2) {
            if (spaces < 2) {
                s = HyoutaUtils::TextUtils::InsertSubstring(s, nidx + 1, "  ", 0, 2 - spaces);
            } else {
                s = HyoutaUtils::TextUtils::Remove(s, nidx + 1, spaces - 2);
            }
        }

        idx = nidx + 1;
    }
    return s;
}

static std::string AdjustNewlinesForQuartz(std::string desc) {
    size_t idx = 0;
    std::string s = std::move(desc);
    while (true) {
        size_t nidx = s.find_first_of('\n', idx);
        if (nidx == std::string::npos) {
            break;
        }
        s = HyoutaUtils::TextUtils::InsertSubstring(s, nidx + 1, "\xe3\x80\x80", 0, 3);
        idx = nidx + 1;
    }
    return s;
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_item.tbl",
            128992,
            HyoutaUtils::Hash::SHA1FromHexString("f3227f619cb17b1403c0de3ad24e816902e0aea5"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

        // An extra newline snuck into the start of Tyrannical Fang's description in Switch v1.0.1
        {
            auto& entry = tbl.Entries[71];
            ItemData m(entry.Data.data(), entry.Data.size());
            m.Description = m.Description.substr(1);
            entry.Data = m.ToBinary();
        }

        // The Switch text has two separate items named Twilight Stone, change the rare one back to
        // the old loc's Dusk Stone.
        {
            auto& entry = tbl.Entries[73];
            ItemData m(entry.Data.data(), entry.Data.size());
            m.Name = "Dusk Stone";
            entry.Data = m.ToBinary();
        }

        // Jack-o'-Lantern has a space and newline at the end of its name, for some reason?
        {
            auto& entry = tbl.Entries[341];
            ItemData m(entry.Data.data(), entry.Data.size());
            m.Name = m.Name.substr(0, m.Name.size() - 2);
            entry.Data = m.ToBinary();
        }

        // normalize newlines, this is a pretty ugly heuristic to catch the right items since we
        // only want to adjust those that start their -- generated or not -- description with the
        // [stats here] block
        for (size_t i = 0; i < 531; ++i) {
            auto& e = tbl.Entries[i];
            if (e.Name == "item") {
                ItemData m(e.Data.data(), e.Data.size());
                if (m.Unknown2[0] != (char)0xc2 && m.Unknown2[0] != (char)0x86
                    && (m.Flags.find_first_of('Z') == std::string::npos
                        || m.Description.starts_with("["))
                    && m.Flags.find_first_of('0') == std::string::npos
                    && m.Description.find_first_of('\n') != std::string::npos) {
                    m.Description = AdjustNewlinesToTwoSpaces(std::move(m.Description));
                    e.Data = m.ToBinary();
                }
            } else if (e.Name == "item_q") {
                ItemData m(e.Data.data(), e.Data.size());
                if (m.Description.find_first_of('\n') != std::string::npos) {
                    m.Description = AdjustNewlinesForQuartz(std::move(m.Description));
                    e.Data = m.ToBinary();
                }
            }
        }

        std::vector<char> bin2;
        HyoutaUtils::Stream::MemoryStream ms(bin2);
        tbl.RecalcNumberOfEntries();
        tbl.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        fileSw->SetVectorData(std::move(bin2));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t_item
