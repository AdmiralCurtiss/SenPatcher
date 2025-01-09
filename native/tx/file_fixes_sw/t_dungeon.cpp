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
__declspec(dllexport) char SenPatcherFix_0_dungeon[] = "Text fixes in dungeon names.";
}

namespace SenLib::TX::FileFixesSw::t_dungeon {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_dungeon.tbl",
            9536,
            HyoutaUtils::Hash::SHA1FromHexString("4ac434996c0d504bc89d1cfa17acf510c007f884"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

        // Second Spirit Barrier -> Second Spiritron Barrier
        {
            auto& entry = tbl.Entries[4];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = (m.Name.substr(0, 13) + "ron" + m.Name.substr(13));
            entry.Data = m.ToBinary();
        }

        // Seventh Spirit Barrier -> Seventh Spiritron Barrier
        {
            auto& entry = tbl.Entries[6];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = (m.Name.substr(0, 14) + "ron" + m.Name.substr(14));
            entry.Data = m.ToBinary();
        }

        // Fourth Spirit Barrier -> Fourth Spiritron Barrier
        {
            auto& entry = tbl.Entries[20];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = (m.Name.substr(0, 13) + "ron" + m.Name.substr(13));
            entry.Data = m.ToBinary();
        }

        // Temple of the Farthest Ends
        {
            auto& entry = tbl.Entries[30];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m7500;
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of Interstices
        {
            auto& entry = tbl.Entries[34];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m8310;
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of Boundaries
        {
            auto& entry = tbl.Entries[35];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m8320;
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of the Afterlife
        {
            auto& entry = tbl.Entries[36];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m8330;
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of the Afterlife (yes this name is in the vanilla t_dungeon twice)
        {
            auto& entry = tbl.Entries[37];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m8340;
            entry.Data = m.ToBinary();
        }

        // Scarlet Battlefield
        {
            auto& entry = tbl.Entries[46];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m3800;
            entry.Data = m.ToBinary();
        }

        // Emerald Waterway
        {
            auto& entry = tbl.Entries[52];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m9810;
            entry.Data = m.ToBinary();
        }

        // Amber Labyrinth (the dungeon in the after story, not the early game dungeon)
        {
            auto& entry = tbl.Entries[53];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m9820;
            entry.Data = m.ToBinary();
        }

        // Ice Demon Road
        {
            auto& entry = tbl.Entries[54];
            DungeonData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m9830;
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
} // namespace SenLib::TX::FileFixesSw::t_dungeon
