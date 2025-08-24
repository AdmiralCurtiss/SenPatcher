#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen3/file_fixes.h"
#include "sen3/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_notemons[] =
    "Fix incorrect place names in monster list.";
}

namespace SenLib::Sen3::FileFixes::t_notemons {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file_en = getCheckedFile(
            "data/text/dat_en/t_notemons.tbl",
            8774,
            HyoutaUtils::Hash::SHA1FromHexString("bf20016491e65cbd7119336e7514615e3cf0b900"));
        if (!file_en) {
            return false;
        }

        Tbl tbl_en(file_en->Data.data(),
                   file_en->Data.size(),
                   HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // "Dark Dragon’s Nest"
        // wrong apostrophe
        {
            auto& e = tbl_en.Entries[30];
            ChapterData m(e.Data.data(), e.Data.size());
            m.name = HyoutaUtils::TextUtils::ReplaceSubstring(m.name, 11, 3, "'");
            e.Data = m.ToBinary();
        }

        std::vector<char> result_en_vec;
        HyoutaUtils::Stream::MemoryStream result_en(result_en_vec);
        tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(
            std::move(result_en_vec), file_en->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t_notemons
