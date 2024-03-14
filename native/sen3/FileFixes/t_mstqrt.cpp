#include <filesystem>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen3/file_fixes.h"
#include "sen3/tbl.h"
#include "sha1.h"
#include "util/text.h"

namespace SenLib::Sen3::FileFixes::t_mstqrt {
std::string_view GetDescription() {
    return "Fix a handful of Master Quartzes having broken effects when playing in English or "
           "French.";
}

static void AttachSentinel(Tbl& tbl, size_t index) {
    auto& l = tbl.Entries[index].Data;
    l.insert(l.begin() + 4, 0x24);
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file_jp = getCheckedFile(
            "data/text/dat/t_mstqrt.tbl",
            31455,
            SenPatcher::SHA1FromHexString("86c4d62ac6efc4ab5dbfdc7f5bf36c4eaef64c3b"));
        auto file_en = getCheckedFile(
            "data/text/dat_en/t_mstqrt.tbl",
            30155,
            SenPatcher::SHA1FromHexString("494b68fcbc463581f79f7dd1f6444df9c4ad6204"));
        auto file_fr = getCheckedFile(
            "data/text/dat_fr/t_mstqrt.tbl",
            30932,
            SenPatcher::SHA1FromHexString("b8022803ece579b5342e5143dc91ae4706fd7f7c"));
        if (!file_jp || !file_en || !file_fr) {
            return false;
        }

        Tbl tbl_jp(file_jp->Data.data(),
                   file_jp->Data.size(),
                   HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        Tbl tbl_en(file_en->Data.data(),
                   file_en->Data.size(),
                   HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        Tbl tbl_fr(file_fr->Data.data(),
                   file_fr->Data.size(),
                   HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // copy over the correct effect data from the JP file
        for (size_t i = 0; i < tbl_jp.Entries.size(); ++i) {
            if (tbl_jp.Entries[i].Name == "MasterQuartzData") {
                tbl_en.Entries[i].Data = tbl_jp.Entries[i].Data;
                tbl_fr.Entries[i].Data = tbl_jp.Entries[i].Data;
            }
        }

        // attach sentinel '$' to the strings where the values need to be swapped
        AttachSentinel(tbl_fr, 67);
        for (Tbl* tbl : {&tbl_en, &tbl_fr}) {
            AttachSentinel(*tbl, 123);
            AttachSentinel(*tbl, 137);
            AttachSentinel(*tbl, 193);
            AttachSentinel(*tbl, 221);
            AttachSentinel(*tbl, 291);
            AttachSentinel(*tbl, 347);
            AttachSentinel(*tbl, 391);
        }

        // also while we're here fix this missing newline in the FR file...
        {
            auto& e = tbl_fr.Entries[66];
            e.Data[e.Data.size() - 2] = 0x0a;
        }

        // Virgo lists EP healing instead of HP
        {
            auto& e = tbl_en.Entries[263];
            MasterQuartzMemo m(e.Data.data(), e.Data.size());
            m.str = Replace(m.str, "EP", "HP");
            e.Data = m.ToBinary();
        }

        // For clarity mention that Breaking is okay too for the 2x drop
        {
            auto& e = tbl_en.Entries[393];
            MasterQuartzMemo m(e.Data.data(), e.Data.size());
            m.str = InsertSubstring(m.str, 54, "breaks or ", 0, 10);
            e.Data = m.ToBinary();
        }

        // Add leading fullwidth space to all MQ effect descriptions, CS4 does this and it looks
        // much nicer.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "MasterQuartzMemo") {
                MasterQuartzMemo m(e.Data.data(), e.Data.size());
                if (!m.str.starts_with("\xe2\x98\x85")) { // black star U+2605
                    m.str = Insert(m.str,
                                   m.str.starts_with("$") ? 1 : 0,
                                   "\xe3\x80\x80"); // fullwidth space U+3000
                    if (i == 191) {
                        m.str = ReplaceSubstring(
                            m.str, m.str.find_last_of('\n') + 1, 1, "\xe3\x80\x80", 0, 3);
                    } else if (i == 393) {
                        m.str = Insert(m.str, m.str.find_last_of('\n') + 1, "\xe3\x80\x80");
                    }
                }
                e.Data = m.ToBinary();
            }
        }

        std::vector<char> result_en_vec;
        MemoryStream result_en(result_en_vec);
        tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        std::vector<char> result_fr_vec;
        MemoryStream result_fr(result_fr_vec);
        tbl_fr.WriteToStream(result_fr, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(
            std::move(result_en_vec), file_en->Filename, SenPatcher::P3ACompressionType::LZ4);
        result.emplace_back(
            std::move(result_fr_vec), file_fr->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t_mstqrt
