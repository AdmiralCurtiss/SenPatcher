#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "util/hash/sha1.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen2::FileFixes::scripts_book_dat_us_book03_dat {
std::string_view GetDescription() {
    return "Consistency with CS1 in Thors library books.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_us/book03.dat",
            17969,
            HyoutaUtils::Hash::SHA1FromHexString("da6f79f56616cb073cc7205f895b1c0d389c0e2d"));
        if (!file) {
            return false;
        }

        using namespace HyoutaUtils::TextUtils;

        auto& bin = file->Data;
        HyoutaUtils::Stream::DuplicatableByteArrayStream s(bin.data(), bin.size());
        BookTable book(s, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // misplaced hyphen
        book.Entries[5].Text = ReplaceSubstring(*book.Entries[5].Text, 279, 1, " ", 0, 1);

        // wording sync with CS1
        book.Entries[5].Text = Remove(*book.Entries[5].Text, 563, 7);
        book.Entries[5].Text =
            ReplaceSubstring(*book.Entries[5].Text, 546, 4, *book.Entries[1].Text, 380, 6);

        // missing word and a bunch of reformatting after that
        book.Entries[34].Text =
            InsertSubstring(*book.Entries[34].Text, 405, *book.Entries[34].Text, 439, 3);
        book.Entries[34].Text = ReplaceSubstring(*book.Entries[34].Text, 444, 1, "\\n", 0, 2);
        book.Entries[34].Text = ReplaceSubstring(*book.Entries[34].Text, 495, 1, "\\n", 0, 2);
        book.Entries[34].Text = ReplaceSubstring(*book.Entries[34].Text, 451, 2, " ", 0, 1);
        book.Entries[35].Text = ReplaceSubstring(*book.Entries[35].Text, 0, 0, " ", 0, 1);
        book.Entries[35].Text =
            ReplaceSubstring(*book.Entries[35].Text, 0, 0, *book.Entries[34].Text, 496, 9);
        book.Entries[34].Text = book.Entries[34].Text->substr(0, 496);
        book.Entries[35].Text = ReplaceSubstring(*book.Entries[35].Text, 51, 1, "\\n", 0, 2);

        // missing start quote
        book.Entries[13].Text = InsertSubstring(*book.Entries[13].Text, 54, "'", 0, 1);

        // book title change in CS1
        book.Entries[37].Text =
            ReplaceSubstring(*book.Entries[37].Text, 620, 15, "Then and Now", 0, 12);

        std::vector<char> bin2;
        HyoutaUtils::Stream::MemoryStream ms(bin2);
        book.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_book_dat_us_book03_dat
