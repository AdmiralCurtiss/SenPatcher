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
__declspec(dllexport) char SenPatcherFix_0_notechar[] = "Text fixes in friend notes.";
}

namespace SenLib::TX::FileFixesSw::t_notechar {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_notechar.tbl",
            73946,
            HyoutaUtils::Hash::SHA1FromHexString("b36f664024b69fcd9752f8cd77c11fd261f94aa2"));
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

        // Switch v1.0.1 decapitalized 'karate team' in the headers, revert that
        // also fix Sora's 'Karate Club' -> 'Karate Team'
        for (int idx : {10, 23, 35, 45}) {
            auto& entry = tbl.Entries[static_cast<size_t>(idx)];
            QSChar m(entry.Data.data(), entry.Data.size());
            for (int sidx : {4, 5}) {
                auto& s = m.Strings[static_cast<size_t>(sidx)];
                s = HyoutaUtils::TextUtils::ReplaceSubstring(s, s.size() - 12, 12, "Karate Team)");
            }
            entry.Data = m.ToBinary();
        }

        // Ayato's 3rd entry has "His hard work culminated a now-second-nature fighting spirit that
        // inspires others." This is missing a 'in' after 'culminated'.
        {
            auto& entry = tbl.Entries[30];
            QSChar m(entry.Data.data(), entry.Data.size());
            m.Strings[10] = HyoutaUtils::TextUtils::Insert(m.Strings[10], 0x18, " in");
            entry.Data = m.ToBinary();
        }

        // Kasumi's 3rd entry is missing a space "shefrantically" -> "she frantically"
        {
            auto& entry = tbl.Entries[92];
            QSChar m(entry.Data.data(), entry.Data.size());
            m.Strings[10] = HyoutaUtils::TextUtils::Insert(m.Strings[10], 0x5b, " ");
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
