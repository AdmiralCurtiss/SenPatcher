#include <string>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen5/tbl.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_notecook[] = "Improve formatting of cooking effects.";
}

namespace SenLib::Sen5::FileFixes::t_notecook {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_en/t_notecook.tbl",
            8683,
            HyoutaUtils::Hash::SHA1FromHexString("c5f63d1aba8498bb43a8b4f8900c790df91c21a1"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // Extra spaces
        {
            auto& e = tbl_en.Entries[1];
            CookData m(e.Data.data(), e.Data.size());
            m.item2line2 = HyoutaUtils::TextUtils::Remove(m.item2line2, 12, 1);
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[4];
            CookData m(e.Data.data(), e.Data.size());
            m.item2line2 = HyoutaUtils::TextUtils::Remove(m.item2line2, 12, 1);
            e.Data = m.ToBinary();
        }

        // replace the separator dot between STR/DEF etc. with a slightly smaller one that's used by
        // the autogenerator, which looks a bit better and is consistent with the autogenerator.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                m.item1line1 =
                    HyoutaUtils::TextUtils::Replace(m.item1line1, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                m.item1line2 =
                    HyoutaUtils::TextUtils::Replace(m.item1line2, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                m.item2line1 =
                    HyoutaUtils::TextUtils::Replace(m.item2line1, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                m.item2line2 =
                    HyoutaUtils::TextUtils::Replace(m.item2line2, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                m.item3line1 =
                    HyoutaUtils::TextUtils::Replace(m.item3line1, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                m.item3line2 =
                    HyoutaUtils::TextUtils::Replace(m.item3line2, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                e.Data = m.ToBinary();
            }
        }


        std::vector<char> result_en_vec;
        HyoutaUtils::Stream::MemoryStream result_en(result_en_vec);
        tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);


        result.emplace_back(
            std::move(result_en_vec), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen5::FileFixes::t_notecook
