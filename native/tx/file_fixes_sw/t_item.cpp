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

namespace SenLib::TX::FileFixesSw::t_item {
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

std::string_view GetDescription() {
    return "Text fixes in item names and descriptions.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_item.tbl",
            128976,
            HyoutaUtils::Hash::SHA1FromHexString("30b862e3deea2a570f9caa5e73f3079826e64b03"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

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
                    m.Description =
                        HyoutaUtils::TextUtils::AdjustNewlinesToTwoSpaces(std::move(m.Description));
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