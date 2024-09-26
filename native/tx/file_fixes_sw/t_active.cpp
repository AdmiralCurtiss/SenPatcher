#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "tx/tbl.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::t_active {
std::string_view GetDescription() {
    return "Text fixes in active voices and voicemails.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_active.tbl",
            135598,
            HyoutaUtils::Hash::SHA1FromHexString("2ce0960f18db6d42bdb90834f0cd8fa54c65bc51"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());


        // Mitsuki's E-Card is grammatically incorrect, "Thank you for ever you've done."
        // Change that to 'all'.
        {
            auto& entry = tbl.Entries[673];
            ActiveVoiceTableData m(entry.Data.data(), entry.Data.size());
            std::swap(m.Text[38], m.Text[44]);
            m.Text = (m.Text.substr(0, 27) + "all" + m.Text.substr(31));
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
