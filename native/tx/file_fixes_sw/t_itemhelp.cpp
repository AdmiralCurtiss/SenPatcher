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
__declspec(dllexport) char SenPatcherFix_0_itemhelp[] =
    "Text fixes in generated item descriptions.";
}

namespace SenLib::TX::FileFixesSw::t_itemhelp {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_itemhelp.tbl",
            5444,
            HyoutaUtils::Hash::SHA1FromHexString("bb331b3a5d7f0e4b3435e905d2548896edc08fa8"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());

        // Unyielding Strength description contains an invalid printf string, I'm amazed the
        // Switch version doesn't crash from that but the PC version most definitely does.
        {
            auto& entry = tbl.Entries[82];
            ItemHelpData m(entry.Data.data(), entry.Data.size());
            std::swap(m.Str[24], m.Str[25]);
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
} // namespace SenLib::TX::FileFixesSw::t_itemhelp
