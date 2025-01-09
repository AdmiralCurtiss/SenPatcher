#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "tx/tbl.h"
#include "util/hash/sha1.h"

#include "dungeon_names.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_place[] = "Text fixes in location names.";
}

namespace SenLib::TX::FileFixesSw::t_place {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_place.tbl",
            15617,
            HyoutaUtils::Hash::SHA1FromHexString("aedd1a3f266cbdefecf3eac1c68ad102a711335b"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

        // Temple of the Farthest Ends
        {
            auto& entry = tbl.Entries[152];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m7500;
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of Interstices
        for (size_t idx : {169u, 170u}) {
            auto& entry = tbl.Entries[idx];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = (DUNGEON_NAME_m8310 + m.Name.substr(m.Name.size() - 2));
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of Boundaries
        for (size_t idx : {171u, 172u}) {
            auto& entry = tbl.Entries[idx];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = (DUNGEON_NAME_m8320 + m.Name.substr(m.Name.size() - 2));
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of Reincarnation
        for (size_t idx : {173u, 174u}) {
            auto& entry = tbl.Entries[idx];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            // yes, this is m8340 despite coming earlier
            m.Name = (DUNGEON_NAME_m8340 + m.Name.substr(m.Name.size() - 2));
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of the Afterlife
        for (size_t idx : {175u, 176u}) {
            auto& entry = tbl.Entries[idx];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            // yes, this is m8330 despite coming later
            m.Name = (DUNGEON_NAME_m8330 + m.Name.substr(m.Name.size() - 2));
            entry.Data = m.ToBinary();
        }

        // Scarlet Battlefield
        for (size_t idx : {269u, 270u, 271u}) {
            auto& entry = tbl.Entries[idx];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = (DUNGEON_NAME_m3800 + m.Name.substr(19));
            entry.Data = m.ToBinary();
        }

        // Emerald Waterway
        {
            auto& entry = tbl.Entries[287];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m9810;
            entry.Data = m.ToBinary();
        }

        // Amber Waterway
        {
            auto& entry = tbl.Entries[288];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m9820;
            entry.Data = m.ToBinary();
        }

        // Ice Demon Road
        {
            auto& entry = tbl.Entries[289];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m9830;
            entry.Data = m.ToBinary();
        }


        // Saves made in the After Story claim they're in the Epilogue
        {
            auto& entry = tbl.Entries[214];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = "After Story";
            entry.Data = m.ToBinary();
        }

        // Normal End save has the wrong chapter name
        {
            auto& entry = tbl.Entries[206];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = "Dusk to Dawn - End";
            entry.Data = m.ToBinary();
        }

        // True End save is missing the special save indicator
        {
            auto& entry = tbl.Entries[205];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = (m.Name.substr(0, 13) + " - End"
                      "\xe3\x80\x80\xe2\x98\x85"
                      "Main Story Clear");
            entry.Data = m.ToBinary();
        }

        // After Story End save is missing the Clear Data indicator
        {
            auto& entry = tbl.Entries[222];
            PlaceTableData m(entry.Data.data(), entry.Data.size());
            m.Name = (m.Name.substr(0, 18) + "-" + m.Name.substr(17) + 
                      "\xe3\x80\x80\xe2\x98\x85"
                      "Clear Data");
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
} // namespace SenLib::TX::FileFixesSw::t_place
