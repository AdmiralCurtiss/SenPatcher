#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "sha1.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book00_dat {
std::string_view GetDescription() {
    return "Minor text fixes in Imperial Chronicle issues.";
}

static void PostSyncFixes(BookTable& book) {
    // a few final fixes so no entries overflow
    for (int idx : {8, 14, 30, 39, 47, 54, 72, 81}) {
        book.Entries[idx].Text =
            HyoutaUtils::TextUtils::Replace(*book.Entries[idx].Text, "\\n\\n", "\\n");
    }
}

static void InjectNewlines(BookTable& book, int index, std::span<const int> linebreaks) {
    auto& e = book.Entries[index];
    auto split = HyoutaUtils::TextUtils::Split(*e.Text, "\\n");
    for (auto it = linebreaks.rbegin(); it != linebreaks.rend(); ++it) {
        split.insert(split.begin() + *it, "");
    }
    e.Text = HyoutaUtils::TextUtils::Join(split, "\\n");
}

static bool IsUpperAscii(char c) {
    return c >= 'A' && c <= 'Z';
}

static size_t GetWhitespaceLength(const char* c, size_t length) {
    if (length >= 1 && (*c == ' ' || *c == '\t' || *c == '\r' || *c == '\f' || *c == '\n')) {
        return 1;
    }
    if (length >= 3 && (*c == '\xe3' && *(c + 1) == '\x80' && *(c + 2) == '\x80')) {
        return 3;
    }
    return 0;
}

static size_t GetWhitespaceLengthFromEnd(std::string_view s) {
    if (s.size() >= 1) {
        char c = s.back();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\n') {
            return 1;
        }
    }
    if (s.size() >= 3) {
        if (s[s.size() - 3] == '\xe3' && s[s.size() - 2] == '\x80' && s[s.size() - 1] == '\x80') {
            return 3;
        }
    }
    return 0;
}

std::string RemoveEndOfStringWhitespace(std::string s) {
    while (true) {
        size_t wslen = GetWhitespaceLengthFromEnd(s);
        if (wslen == 0) {
            return s;
        }
        for (size_t i = 0; i < wslen; ++i) {
            s.pop_back();
        }
    }
    return s;
}

static bool IsWhitespaceOnlyForNewspaper(std::string_view s) {
    size_t startCheckIndex = s.starts_with("#") ? 5 : 0;
    size_t i = startCheckIndex;
    while (i < s.size()) {
        size_t wslen = GetWhitespaceLength(&s[i], s.size() - i);
        if (wslen == 0) {
            return false;
        }
        i += wslen;
    }
    return true;
}

static void CleanUpWhitespace(BookTable& book) {
    for (size_t i = 0; i < book.Entries.size(); ++i) {
        auto& e = book.Entries[i];
        if (e.Text.has_value()) {
            std::vector<std::unique_ptr<std::string>> tmp;
            auto split = HyoutaUtils::TextUtils::Split(*e.Text, "\\n");
            for (size_t j = 1; j < split.size(); ++j) {
                if (split[j].find("[") != std::string_view::npos
                    || split[j].find("\xe2\x97\x86") != std::string_view::npos
                    || split[j].find("\xe2\x94\x81") != std::string_view::npos
                    || (split[j].size() > 5 && split[j].ends_with(":")
                        && IsUpperAscii(split[j][5]))) {
                    if (!IsWhitespaceOnlyForNewspaper(split[j - 1])) {
                        split.insert(split.begin() + j, "");
                        ++j;
                    }
                }
            }
            for (size_t j = 0; j < split.size(); ++j) {
                if (IsWhitespaceOnlyForNewspaper(split[j])) {
                    split[j] = "";
                } else if (GetWhitespaceLengthFromEnd(split[j]) != 0) {
                    auto tmpstr = std::make_unique<std::string>(
                        RemoveEndOfStringWhitespace(std::string(split[j])));
                    split[j] = *tmpstr;
                    tmp.emplace_back(std::move(tmpstr));
                }
            }
            for (size_t j = split.size(); j > 0; --j) {
                if (IsWhitespaceOnlyForNewspaper(split[j - 1])) {
                    split.erase(split.begin() + j - 1);
                } else {
                    break;
                }
            }
            e.Text = HyoutaUtils::TextUtils::Join(split, "\\n");
        }
    }
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_us/book00.dat",
            45024,
            SenPatcher::SHA1FromHexString("2eca835428184ad35a9935dc5d2deaa60d444aad"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::TextUtils;

        DuplicatableByteArrayStream s(bin.data(), bin.size());
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
        MemoryStream ms(bin2);
        book.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book00_dat
