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
__declspec(dllexport) char SenPatcherFix_0_notehelp[] =
    "Sync help entries with what the PC version expects.";
}

namespace SenLib::TX::FileFixesSw::t_notehelp {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        SenPatcher::P3APackFile* fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_notehelp.tbl",
            2598,
            HyoutaUtils::Hash::SHA1FromHexString("f09118bda1b7008bbbbda4aced2ea87469b2f53f"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());
        tbl.Entries.erase(tbl.Entries.begin() + 4);
        tbl.Entries.erase(tbl.Entries.begin() + 17);

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
} // namespace SenLib::TX::FileFixesSw::t_notehelp
