#pragma once

#include <string>
#include <string_view>

#include "sen/book_table.h"
#include "util/text.h"

namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book00_dat {
static void PostSyncFixes(BookTable& book) {
    // a few final fixes so no entries overflow
    for (int idx : {8, 14, 30, 39, 47, 54, 72, 81}) {
        book.Entries[idx].Text =
            HyoutaUtils::TextUtils::Replace(*book.Entries[idx].Text, "\\n\\n", "\\n");
    }
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

static std::string RemoveEndOfStringWhitespace(std::string s) {
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
} // namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book00_dat
