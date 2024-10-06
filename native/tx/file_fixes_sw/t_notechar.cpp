#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "tx/tbl.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::t_notechar {
std::string_view GetDescription() {
    return "Text fixes in friend notes.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_notechar.tbl",
            73887,
            HyoutaUtils::Hash::SHA1FromHexString("886ce018e013ba3d9b5d6f52b2fa53d93d543ec8"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

        // drop the colon after Class in all the students' first entries
        for (size_t i = 0; i < tbl.Entries.size(); ++i) {
            auto& entry = tbl.Entries[i];
            if (entry.Name == "QSChar") {
                QSChar m(entry.Data.data(), entry.Data.size());
                for (size_t j = 0; j < m.Strings.size(); ++j) {
                    if (m.Strings[j].starts_with("#599cClass:")) {
                        m.Strings[j][10] = ' ';
                    }
                }
                entry.Data = m.ToBinary();
            }
        }

        // Nodoka entry 4 gets very close to the edge of the textbox
        {
            auto& entry = tbl.Entries[29];
            QSChar m(entry.Data.data(), entry.Data.size());
            std::swap(m.Strings[12][122], m.Strings[12][126]);
            entry.Data = m.ToBinary();
        }

        // Ayato entry 2 escapes textbox
        {
            auto& entry = tbl.Entries[30];
            QSChar m(entry.Data.data(), entry.Data.size());
            m.Strings[8][74] = '\n';
            entry.Data = m.ToBinary();
        }

        // Shige entry 3 escapes textbox
        {
            auto& entry = tbl.Entries[95];
            QSChar m(entry.Data.data(), entry.Data.size());
            std::swap(m.Strings[10][75], m.Strings[10][86]);
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
} // namespace SenLib::TX::FileFixesSw::t_notechar
