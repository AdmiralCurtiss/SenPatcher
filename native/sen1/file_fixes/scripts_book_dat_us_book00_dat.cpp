#include <span>
#include <string_view>
#include <vector>

#include "sen1/file_fixes/scripts_book_dat_us_book00_dat_shared.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book00_dat {
std::string_view GetDescription() {
    return "Minor text fixes in Imperial Chronicle issues.";
}

static void InjectNewlines(BookTable& book, int index, std::span<const int> linebreaks) {
    auto& e = book.Entries[index];
    auto split = HyoutaUtils::TextUtils::Split(*e.Text, "\\n");
    for (auto it = linebreaks.rbegin(); it != linebreaks.rend(); ++it) {
        split.insert(split.begin() + *it, "");
    }
    e.Text = HyoutaUtils::TextUtils::Join(split, "\\n");
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_us/book00.dat",
            45024,
            HyoutaUtils::Hash::SHA1FromHexString("2eca835428184ad35a9935dc5d2deaa60d444aad"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::TextUtils;

        HyoutaUtils::Stream::DuplicatableByteArrayStream s(bin.data(), bin.size());
        BookTable book(s, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // fix inconsistent indentation
        book.Entries[43].Text = Insert(*book.Entries[43].Text, 209, " ");
        book.Entries[51].Text = Insert(*book.Entries[51].Text, 55, " ");
        book.Entries[52].Text = Insert(*book.Entries[52].Text, 97, " ");

        // capitalization consistency with main game script
        book.Entries[8].Text = ReplaceSubstring(*book.Entries[8].Text, 189, 1, "p", 0, 1);
        book.Entries[19].Text = ReplaceSubstring(*book.Entries[19].Text, 461, 1, "p", 0, 1);
        book.Entries[51].Text = ReplaceSubstring(*book.Entries[51].Text, 63, 1, "p", 0, 1);
        book.Entries[58].Text = ReplaceSubstring(*book.Entries[58].Text, 442, 1, "p", 0, 1);
        book.Entries[72].Text = ReplaceSubstring(*book.Entries[72].Text, 301, 1, "p", 0, 1);

        // weirdly formatted sub-headline
        book.Entries[3].Text = ReplaceSubstring(*book.Entries[3].Text, 37, 1, "\xe2\x97\x86", 0, 3);
        book.Entries[3].Text = InsertSubstring(*book.Entries[3].Text, 5, "\xe2\x97\x86", 0, 3);
        book.Entries[3].Text =
            ReplaceSubstring(*book.Entries[3].Text, 0, 5, *book.Entries[2].Text, 0, 5);

        // insert linebreaks before headlines like CS2 does
        // and clean up whitespace for easier diffing
        CleanUpWhitespace(book);

        // the above misses a handful of linebreaks that are hard to autodetect, manually inject
        // those
        InjectNewlines(book, 31, {{1, 5}});
        InjectNewlines(book, 34, {{5, 9}});
        InjectNewlines(book, 35, {{4, 7}});
        InjectNewlines(book, 36, {{4}});
        InjectNewlines(book, 37, {{6}});
        InjectNewlines(book, 63, {{8}});
        InjectNewlines(book, 64, {{6}});
        InjectNewlines(book, 65, {{5, 6}});
        InjectNewlines(book, 66, {{7}});
        InjectNewlines(book, 85, {{2}});

        PostSyncFixes(book);

        std::vector<char> bin2;
        HyoutaUtils::Stream::MemoryStream ms(bin2);
        book.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book00_dat
