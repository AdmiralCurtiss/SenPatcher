#include <array>
#include <cstring>
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
__declspec(dllexport) char SenPatcherFix_0_text[] =
    "Sync text IDs with what the PC version expects and fix a few issues.";
}

namespace SenLib::TX::FileFixesSw::t_text {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto filePc = getCheckedFile(
            "text/dat/t_text.tbl",
            31946,
            HyoutaUtils::Hash::SHA1FromHexString("0c0b88aad59b3482638cd588ba460c79145b4a98"));
        SenPatcher::P3APackFile* fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_text.tbl",
            32079,
            HyoutaUtils::Hash::SHA1FromHexString("298c265762d5ef233fc999aa04261619a869ee46"));
        if (!filePc || !fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tblSw(bin.data(), bin.size());
        SenLib::TX::Tbl tblPc(filePc->Data.data(), filePc->Data.size());

        tblSw.Entries.erase(tblSw.Entries.begin() + 75);
        tblSw.Entries.erase(tblSw.Entries.begin() + 417);
        tblSw.Entries.erase(tblSw.Entries.begin() + 558);
        tblSw.Entries.erase(tblSw.Entries.begin() + 558);
        tblSw.Entries.erase(tblSw.Entries.begin() + 558);
        tblSw.Entries.erase(tblSw.Entries.begin() + 759);
        for (size_t i = 0; i < tblSw.Entries.size(); ++i) {
            TextTableData m(tblSw.Entries[i].Data.data(), tblSw.Entries[i].Data.size());
            if (m.Idx > 0x64) {
                m.Idx -= 1;
            }
            if (m.Idx > 0x229) {
                m.Idx -= 1;
            }
            if (m.Idx > 0x264) {
                m.Idx -= 3;
            }
            if (m.Idx > 0x348) {
                m.Idx -= 1;
            }
            tblSw.Entries[i].Data = m.ToBinary();
        }
        tblSw.Entries.push_back(tblPc.Entries[761]);
        tblSw.Entries.push_back(tblPc.Entries[762]);

        // name of the grade shop points. eX+ does not have these anymore (leftover from Vita, I
        // think?), but change this from Continue Point to NG+ Point (what CS1/2 uses)
        {
            auto& entry = tblSw.Entries[666];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = "NG+ Point";
            entry.Data = m.ToBinary();
        }

        // the NG+ description is complete nonsense, write a better one
        {
            auto& entry = tblSw.Entries[667];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = (m.Str.substr(0, 3) + "Select what to carry over from your Clear Data.");
            entry.Data = m.ToBinary();
        }

        // the textbox you get when loading a clear data still talks about the NG+ point mechanic
        // (the JP version of the same string doesn't), so remove that.
        {
            auto& entry = tblSw.Entries[558];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = (m.Str.substr(0, 54) + ".\nSome bonus features are also available.");
            entry.Data = m.ToBinary();
        }

        // Add a space for the Skill upgrade status messages, otherwise you end up with stuff like
        // "Ranged SkillSplash Arrow"
        for (size_t idx : {285u, 286u, 287u}) {
            auto& entry = tblSw.Entries[idx];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = (" " + m.Str);
            entry.Data = m.ToBinary();
        }

        // make the treasure chest labels more clear
        {
            auto& entry = tblSw.Entries[702];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = (m.Str.substr(0, 5) + "Treasure Chest (Previously opened)");
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tblSw.Entries[703];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = (m.Str.substr(0, 5) + "Treasure Chest (Never opened)");
            entry.Data = m.ToBinary();
        }

        // sync Auto-Recover and Brave Soul skill descriptions (when unacquired), see also t_magic
        {
            auto& entry = tblSw.Entries[678];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = (m.Str.substr(0, 31)
                     + "As Support/Partner, restore active character's HP upon their KO.");
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tblSw.Entries[679];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = (m.Str.substr(0, 31) + "Automatically restore HP upon KO.");
            entry.Data = m.ToBinary();
        }

        // stray period in Always Off option for EX Skill cut-ins
        {
            auto& entry = tblSw.Entries[695];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = m.Str.substr(0, m.Str.size() - 1);
            entry.Data = m.ToBinary();
        }

        // Epilogue -> Side/After Story for the option in the boss refight menu
        {
            auto& entry = tblSw.Entries[735];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = m.Str.substr(0, 5) + "Side/After Story";
            entry.Data = m.ToBinary();
        }

        std::vector<char> bin2;
        HyoutaUtils::Stream::MemoryStream ms(bin2);
        tblSw.RecalcNumberOfEntries();
        tblSw.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        fileSw->SetVectorData(std::move(bin2));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t_text
