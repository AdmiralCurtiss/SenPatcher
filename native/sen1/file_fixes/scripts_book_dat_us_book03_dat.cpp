#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "sha1.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book03_dat {
std::string_view GetDescription() {
    return "Minor text fixes in Thors library books.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_us/book03.dat",
            18557,
            SenPatcher::SHA1FromHexString("9ed835634edf89fff1757d4fa22e4a1506ca0f04"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::TextUtils;

        DuplicatableByteArrayStream s(bin.data(), bin.size());
        BookTable book(s, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // misplaced hyphen
        book.Entries[5].Text = ReplaceSubstring(*book.Entries[5].Text, 279, 1, " ", 0, 1);

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

        // double space
        book.Entries[14].Text = Remove(*book.Entries[14].Text, 288, 1);

        // capitalization consistency with main game script
        book.Entries[2].Text = ReplaceSubstring(*book.Entries[2].Text, 41, 1, "p", 0, 1);

        std::vector<char> bin2;
        MemoryStream ms(bin2);
        book.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book03_dat
