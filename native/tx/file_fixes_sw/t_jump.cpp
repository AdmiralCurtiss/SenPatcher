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
__declspec(dllexport) char SenPatcherFix_0_jump[] = "Text fixes in navigation menu.";
}

namespace SenLib::TX::FileFixesSw::t_jump {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_jump.tbl",
            7778,
            HyoutaUtils::Hash::SHA1FromHexString("74acb57793fc439eface6744bf0e0e84139b7ad7"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

        // Temple of the Farthest Ends
        {
            auto& entry = tbl.Entries[94];
            MapJumpData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m7500;
            entry.Data = m.ToBinary();
        }

        // Box - Path of Interstices
        {
            auto& entry = tbl.Entries[102];
            MapJumpData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m8310;
            entry.Data = m.ToBinary();
        }

        // Box - Path of Boundaries
        {
            auto& entry = tbl.Entries[103];
            MapJumpData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m8320;
            entry.Data = m.ToBinary();
        }

        // Box - Path of Reincarnation
        {
            auto& entry = tbl.Entries[104];
            MapJumpData m(entry.Data.data(), entry.Data.size());
            // yes, this is m8340 despite coming earlier
            m.Name = DUNGEON_NAME_m8340;
            entry.Data = m.ToBinary();
        }

        // Box - Path of the Afterlife
        {
            auto& entry = tbl.Entries[105];
            MapJumpData m(entry.Data.data(), entry.Data.size());
            // yes, this is m8330 despite coming later
            m.Name = DUNGEON_NAME_m8330;
            entry.Data = m.ToBinary();
        }

        // Box - Final Corridor
        {
            auto& entry = tbl.Entries[106];
            MapJumpData m(entry.Data.data(), entry.Data.size());
            m.Name = ("Pandora" + m.Name.substr(3));
            entry.Data = m.ToBinary();
        }

        // Scarlet Battlefield
        {
            auto& entry = tbl.Entries[113];
            MapJumpData m(entry.Data.data(), entry.Data.size());
            m.Name = DUNGEON_NAME_m3800;
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
} // namespace SenLib::TX::FileFixesSw::t_jump
