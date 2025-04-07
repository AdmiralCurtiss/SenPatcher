#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "tx/tbl.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_active[] = "Text fixes in active voices and voicemails.";
}

namespace SenLib::TX::FileFixesSw::t_active {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_active.tbl",
            135611,
            HyoutaUtils::Hash::SHA1FromHexString("5ada6567d5c490f5caab2199a7ddd1ff0baf904d"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());


        // Mitsuki's E-Card is grammatically incorrect, "Thank you for ever you've done."
        // Change that to 'all'.
        // See also I_VIS305, which is the same message in graphic form.
        // Switch v1.0.1 fixes the graphic but leaves the text unfixed...
        {
            auto& entry = tbl.Entries[673];
            ActiveVoiceTableData m(entry.Data.data(), entry.Data.size());
            std::swap(m.Text[38], m.Text[44]);
            m.Text = (m.Text.substr(0, 27) + "all" + m.Text.substr(31));
            entry.Data = m.ToBinary();
        }

        // "God dam it" -> "God damn it"
        {
            auto& entry = tbl.Entries[549];
            ActiveVoiceTableData m(entry.Data.data(), entry.Data.size());
            m.Text = HyoutaUtils::TextUtils::Insert(m.Text, 7, "n");
            entry.Data = m.ToBinary();
        }

        // "magestic" is not a word, presumably they meant "majestic"?
        // Also while we're here change the unicode ellipsis to three dots.
        {
            auto& entry = tbl.Entries[591];
            ActiveVoiceTableData m(entry.Data.data(), entry.Data.size());
            m.Text = HyoutaUtils::TextUtils::ReplaceSubstring(m.Text, 49, 1, "j");
            m.Text = HyoutaUtils::TextUtils::ReplaceSubstring(m.Text, 31, 3, "...");
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
} // namespace SenLib::TX::FileFixesSw::t_active
