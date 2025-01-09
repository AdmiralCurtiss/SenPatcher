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
__declspec(dllexport) char SenPatcherFix_0_notemons[] = "Text fixes in enemy list.";
}

namespace SenLib::TX::FileFixesSw::t_notemons {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_notemons.tbl",
            13466,
            HyoutaUtils::Hash::SHA1FromHexString("50f44e52868bc93a382629a09ab5386dbb4df765"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

        // Temple of the Farthest Ends
        {
            auto& entry = tbl.Entries[27];
            QSStage m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m7500;
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of Interstices
        {
            auto& entry = tbl.Entries[35];
            QSStage m(entry.Data.data(), entry.Data.size());
            m.Name = (DUNGEON_NAME_m8310 + m.Name.substr(m.Name.size() - 2));
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of Boundaries
        {
            auto& entry = tbl.Entries[36];
            QSStage m(entry.Data.data(), entry.Data.size());
            m.Name = (DUNGEON_NAME_m8320 + m.Name.substr(m.Name.size() - 2));
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of the Afterlife
        {
            auto& entry = tbl.Entries[37];
            QSStage m(entry.Data.data(), entry.Data.size());
            // yes, this is m8340 despite coming earlier
            // and yes, the vanilla file swapped the two names here
            m.Name = DUNGEON_NAME_m8340;
            entry.Data = m.ToBinary();
        }

        // Pandora - Path of Reincarnation
        {
            auto& entry = tbl.Entries[38];
            QSStage m(entry.Data.data(), entry.Data.size());
            // yes, this is m8330 despite coming later
            // and yes, the vanilla file swapped the two names here
            m.Name = DUNGEON_NAME_m8330;
            entry.Data = m.ToBinary();
        }

        // Scarlet Battlefield
        {
            auto& entry = tbl.Entries[48];
            QSStage m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m3800;
            entry.Data = m.ToBinary();
        }

        // Emerald Waterway
        {
            auto& entry = tbl.Entries[54];
            QSStage m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m9810;
            entry.Data = m.ToBinary();
        }

        // Amber Waterway
        {
            auto& entry = tbl.Entries[55];
            QSStage m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m9820;
            entry.Data = m.ToBinary();
        }

        // Ice Demon Road
        {
            auto& entry = tbl.Entries[56];
            QSStage m(entry.Data.data(), entry.Data.size());
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
} // namespace SenLib::TX::FileFixesSw::t_notemons
