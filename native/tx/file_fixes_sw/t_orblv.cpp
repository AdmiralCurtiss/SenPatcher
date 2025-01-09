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
__declspec(dllexport) char SenPatcherFix_0_orblv[] = "Text fixes in grid upgrade screen.";
}

namespace SenLib::TX::FileFixesSw::t_orblv {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_orblv.tbl",
            19516,
            HyoutaUtils::Hash::SHA1FromHexString("faa387fd5157c20b787c5737ed21b1805f38dff9"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

        // same fixes as t_magic, but with slightly different formatting because of text box size
        // constraints
        {
            auto& entry = tbl.Entries[370];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str =
                "As Support or Partner, restore\n"
                "20% of active character's HP upon\n"
                "their KO (once per labyrinth)";
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[371];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str =
                "As Support or Partner, restore\n"
                "50% of active character's HP upon\n"
                "their KO (once per labyrinth)";
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[372];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str =
                "As Support or Partner, restore\n"
                "all of active character's HP upon\n"
                "their KO (once per labyrinth)";
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[373];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str =
                "Automatically restore 1 HP\n"
                "upon KO (once per labyrinth)";
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[374];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str =
                "Automatically restore 30%\n"
                "of your HP upon KO (once per\n"
                "labyrinth)";
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[375];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str =
                "Automatically restore all\n"
                "of your HP upon KO (once per\n"
                "labyrinth)";
            entry.Data = m.ToBinary();
        }
        for (size_t idx : {376u, 377u, 378u}) {
            auto& entry = tbl.Entries[idx];
            TextTableData m(entry.Data.data(), entry.Data.size());
            m.Str = ("Increase damage dealt" + m.Str.substr(22, m.Str.size() - 23));
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
} // namespace SenLib::TX::FileFixesSw::t_orblv
