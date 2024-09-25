#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "tx/tbl.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::t_hikitugi {
std::string_view GetDescription() {
    return "Text fixes in New Game+ transfer options.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_hikitugi.tbl",
            1602,
            HyoutaUtils::Hash::SHA1FromHexString("ee0889003da42693079b1c9bf7e017420207b232"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

        // remove the weird spaces for all of the entries
        for (size_t i = 0; i < 7; ++i) {
            auto& entry = tbl.Entries[i];
            hkitugi_lst m(entry.Data.data(), entry.Data.size());
            m.name = (m.name.substr(0, 13) + m.name.substr(14));
            entry.Data = m.ToBinary();
        }
        for (size_t i = 7; i < 12; ++i) {
            auto& entry = tbl.Entries[i];
            hkitugi_lst m(entry.Data.data(), entry.Data.size());
            m.name = (m.name.substr(0, 3) + m.name.substr(4, 5) + m.name.substr(10));
            entry.Data = m.ToBinary();
        }

        // center the Start Game text
        {
            auto& entry = tbl.Entries[12];
            hkitugi_lst m(entry.Data.data(), entry.Data.size());
            m.name = (m.name.substr(0, 27) + " " + m.name.substr(33));
            entry.Data = m.ToBinary();
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
} // namespace SenLib::TX::FileFixesSw::t_hikitugi
