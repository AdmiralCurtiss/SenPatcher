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
__declspec(dllexport) char SenPatcherFix_0_magic[] = "Text fixes in skill descriptions.";
}

namespace SenLib::TX::FileFixesSw::t_magic {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_magic.tbl",
            22622,
            HyoutaUtils::Hash::SHA1FromHexString("6cea9488e2093d9cf1e2d09f0c18eea57e5208a6"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

        // the Auto-Recover and Brave Soul skills have confusing descriptions compared to how they
        // actually work, improve those
        {
            auto& entry = tbl.Entries[111];
            MagicData m(entry.Data.data(), entry.Data.size());
            m.Description =
                "As Support or Partner, automatically restore 20% of\n"
                "active character's HP upon their KO (once per labyrinth).";
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[112];
            MagicData m(entry.Data.data(), entry.Data.size());
            m.Description =
                "As Support or Partner, automatically restore 50% of\n"
                "active character's HP upon their KO (once per labyrinth).";
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[113];
            MagicData m(entry.Data.data(), entry.Data.size());
            m.Description =
                "As Support or Partner, automatically restore all of\n"
                "active character's HP upon their KO (once per labyrinth).";
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[115];
            MagicData m(entry.Data.data(), entry.Data.size());
            m.Description =
                "Automatically restore 1 HP upon KO\n"
                "(once per labyrinth).";
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[116];
            MagicData m(entry.Data.data(), entry.Data.size());
            m.Description =
                "Automatically restore 30% of your HP upon KO\n"
                "(once per labyrinth).";
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[117];
            MagicData m(entry.Data.data(), entry.Data.size());
            m.Description =
                "Automatically restore all of your HP upon KO\n"
                "(once per labyrinth).";
            entry.Data = m.ToBinary();
        }

        // the Boost Combo skill is also slightly weirdly worded and three-line instead of two-line,
        // fix that
        {
            auto& entry = tbl.Entries[119];
            MagicData m(entry.Data.data(), entry.Data.size());
            m.Description = HyoutaUtils::TextUtils::Replace(m.Description, "\n", " ");
            m.Description[45] = '\n';
            m.Description = ("Increase damage dealt" + m.Description.substr(22));
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[120];
            MagicData m(entry.Data.data(), entry.Data.size());
            m.Description = HyoutaUtils::TextUtils::Replace(m.Description, "\n", " ");
            m.Description[45] = '\n';
            m.Description = ("Increase damage dealt" + m.Description.substr(22));
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[121];
            MagicData m(entry.Data.data(), entry.Data.size());
            m.Description = HyoutaUtils::TextUtils::Replace(m.Description, "\n", " ");
            m.Description[46] = '\n';
            m.Description = ("Increase damage dealt" + m.Description.substr(22));
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
} // namespace SenLib::TX::FileFixesSw::t_magic
