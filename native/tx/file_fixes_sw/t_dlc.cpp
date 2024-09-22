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

namespace SenLib::TX::FileFixesSw::t_dlc {
std::string_view GetDescription() {
    return "Reinject PC-only DLC table entries.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto filePc = getCheckedFile(
            "text/dat/t_dlc.tbl",
            9980,
            HyoutaUtils::Hash::SHA1FromHexString("7dfece8709fdcbc2ba9526a7a98379d744035f92"));
        SenPatcher::P3APackFile* fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_dlc.tbl",
            9800,
            HyoutaUtils::Hash::SHA1FromHexString("31f0a279a2962078533c682a819ca27b6e47d2a9"));
        if (!filePc || !fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tblSw(bin.data(), bin.size());
        SenLib::TX::Tbl tblPc(filePc->Data.data(), filePc->Data.size());

        std::vector<TblEntry> dlcEntries;
        dlcEntries.reserve(60);
        dlcEntries.insert(dlcEntries.end(), tblSw.Entries.begin(), tblSw.Entries.begin() + 39);
        dlcEntries.insert(dlcEntries.end(), tblPc.Entries.begin() + 39, tblPc.Entries.begin() + 43);
        dlcEntries.insert(dlcEntries.end(), tblSw.Entries.begin() + 39, tblSw.Entries.begin() + 42);
        dlcEntries.insert(dlcEntries.end(), tblPc.Entries.begin() + 46, tblPc.Entries.begin() + 49);
        dlcEntries.insert(dlcEntries.end(), tblSw.Entries.begin() + 42, tblSw.Entries.end());
        tblSw.Entries = std::move(dlcEntries);

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
} // namespace SenLib::TX::FileFixesSw::t_dlc
