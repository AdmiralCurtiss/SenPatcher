#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen1/tbl.h"
#include "util/hash/sha1.h"

#include "text_dat_us_t_item_tbl_t_magic_tbl.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_notecook[] =
    "Adjust formatting in cooking descriptions.";
}

namespace SenLib::Sen1::FileFixes::text_dat_us_t_notecook_tbl {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_us/t_notecook.tbl",
            4674,
            HyoutaUtils::Hash::SHA1FromHexString("a3e1c05218c7059c371f3641a1111e166e899087"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        Tbl tbl(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        for (TblEntry& entry : tbl.Entries) {
            if (entry.Name == "QSCook") {
                using text_dat_us_t_item_tbl_t_magic_tbl::FixHpEpCpSpacing;

                CookData item(entry.Data.data(), entry.Data.size());
                item.item1line1 = FixHpEpCpSpacing(std::move(item.item1line1));
                item.item1line2 = FixHpEpCpSpacing(std::move(item.item1line2));
                item.item2line1 = FixHpEpCpSpacing(std::move(item.item2line1));
                item.item2line2 = FixHpEpCpSpacing(std::move(item.item2line2));
                item.item3line1 = FixHpEpCpSpacing(std::move(item.item3line1));
                item.item3line2 = FixHpEpCpSpacing(std::move(item.item3line2));
                item.item4line1 = FixHpEpCpSpacing(std::move(item.item4line1));
                item.item4line2 = FixHpEpCpSpacing(std::move(item.item4line2));
                entry.Data = item.ToBinary();
            }
        }

        std::vector<char> bin2;
        HyoutaUtils::Stream::MemoryStream ms(bin2);
        tbl.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::text_dat_us_t_notecook_tbl
