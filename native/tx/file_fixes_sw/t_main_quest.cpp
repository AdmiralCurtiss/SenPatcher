#include <array>
#include <string_view>
#include <utility>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "tx/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_main[] = "Text fixes in shop info and story synopsis.";
__declspec(dllexport) char SenPatcherFix_0_quest[] = "Text fixes in quest log.";
}

static std::string ReformatStoryLine(std::string_view line, size_t questAuthorPrefixCount) {
    std::string_view l = line;
    std::string r;

    if (l.starts_with("#666c")) {
        // this is a separator between the quest description and the quest actions, leave
        // unchanged
        r.append(l);
        return r;
    }

    // drop leading formatting
    size_t dashes = 0;
    bool hadStar = false;
    while (!l.empty()) {
        if (l.front() == ' ') {
            l = l.substr(1);
            continue;
        }
        if (l.front() == '-') {
            ++dashes;
            l = l.substr(1);
            continue;
        }
        if (l.starts_with("\xe3\x80\x80")) {
            l = l.substr(3);
            continue;
        }
        if (l.starts_with("\xe2\x98\x86")) {
            hadStar = true;
            l = l.substr(3);
            continue;
        }
        break;
    }

    if (!l.empty()) {
        if (l.front() == '#') {
            // this is a date at the very top of an entry
            r.append(" ");
            r.append(l);
        } else if (dashes == 2 && questAuthorPrefixCount > 0) {
            // this is the author of a quest request
            for (size_t i = 0; i < questAuthorPrefixCount; ++i) {
                r.append("\xe3\x80\x80");
            }
            r.append("--");
            r.append(l);
        } else if (dashes > 0) {
            // this is the start of a single entry
            r.append("-\xe3\x80\x80");
            r.append(l);
        } else if (hadStar) {
            // this is an entry where you got bonus virtue or whatever
            r.append("\xe2\x98\x86 ");
            r.append(l);
        } else {
            // this is a continuation of a single entry
            r.append(" \xe3\x80\x80");
            r.append(l);
        }
    }

    while (!r.empty() && r.back() == ' ') {
        r.pop_back();
    }

    return r;
}

namespace SenLib::TX::FileFixesSw::t_main {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_main.tbl",
            95848,
            HyoutaUtils::Hash::SHA1FromHexString("193751e5af906163543e4cc335764029a45d4c3f"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());
        using namespace HyoutaUtils::TextUtils;

        // TODO: It would be nice to make the shop infos consistently aligned, but that is hard
        // with a variable width font. Are there commands for absolute text positioning or
        // something like that?

        // formatting in Chapter 1 shop info
        {
            auto& entry = tbl.Entries[14];
            QSText m(entry.Data.data(), entry.Data.size());
            auto lines = Split(m.Text, "\n");
            std::vector<std::string> s;
            s.resize(lines.size());

            {
                static constexpr size_t i = 2;
                s[i] = Insert(lines[i], 3, "\xe3\x80\x80");
                lines[i] = s[i];
            }

            m.Text = Join(lines, "\n");
            entry.Data = m.ToBinary();
        }

        // Uncapitalized 'k' in "Intermediate Karate" item name in Chapter 2 Shop Info
        {
            auto& entry = tbl.Entries[16];
            QSText m(entry.Data.data(), entry.Data.size());
            m.Text[185] = 'K';
            entry.Data = m.ToBinary();
        }

        // Missing color tag in Chapter 7 Shop Info that hides the 'F' in 'Flight Jacket'
        {
            auto& entry = tbl.Entries[26];
            QSText m(entry.Data.data(), entry.Data.size());
            m.Text = Insert(m.Text, 0x58c, "c");
            entry.Data = m.ToBinary();
        }

        // formatting in Final Chapter shop info
        {
            auto& entry = tbl.Entries[28];
            QSText m(entry.Data.data(), entry.Data.size());
            m.Text = Remove(m.Text, 0x25, 6);
            m.Text = Insert(m.Text, 0x441, "\xe2\x94\x81");
            entry.Data = m.ToBinary();
        }

        // formatting in Epilogue shop info
        {
            auto& entry = tbl.Entries[30];
            QSText m(entry.Data.data(), entry.Data.size());
            m.Text = ReplaceSubstring(m.Text, 0x7d, 3, "  ");
            m.Text = Remove(m.Text, 0x25, 6);
            m.Text = Insert(m.Text, 0xff, "\xe2\x94\x81");
            entry.Data = m.ToBinary();
        }

        // After Story shop info
        {
            auto& entry = tbl.Entries[32];
            QSText m(entry.Data.data(), entry.Data.size());
            m.Text.erase(m.Text.begin() + 0x1d0);             // extra C in the middle of the text
            m.Text = ReplaceSubstring(m.Text, 0x350, 3, "s"); // Featured Shop -> Featured Shops
            entry.Data = m.ToBinary();
        }

        // Chapter 2 "In Search of Aizawa"
        {
            auto& entry = tbl.Entries[61];
            QSText m(entry.Data.data(), entry.Data.size());
            // "If her emotions kept spiraling, she got spirited away by the Otherworld."
            // this is nonsense, change that to
            // "Her emotions kept spiraling, so she got spirited away by the Otherworld."
            m.Text = Insert(m.Text, 140, "so ");
            m.Text = ReplaceSubstring(m.Text, 108, 4, "H");
            entry.Data = m.ToBinary();
        }

        // "Location Q" -> "Point Q" to match the actual in-game dialogue and images
        {
            auto& entry = tbl.Entries[212];
            QSText m(entry.Data.data(), entry.Data.size());
            m.Text = ReplaceSubstring(m.Text, 0x97, 8, "Point");
            entry.Data = m.ToBinary();
        }
        {
            auto& entry = tbl.Entries[213];
            QSText m(entry.Data.data(), entry.Data.size());
            m.Text = ReplaceSubstring(m.Text, 0x8c, 8, "Point");
            entry.Data = m.ToBinary();
        }

        // This line is too long and escapes the window, move linebreak
        {
            auto& entry = tbl.Entries[207];
            QSText m(entry.Data.data(), entry.Data.size());
            HyoutaUtils::Vector::ShiftData(m.Text, 0x2ce, 0x2c2, 4);
            entry.Data = m.ToBinary();
        }

        // helper stuff for the shop info
        // for (size_t i = 0; i < 34; ++i) {
        //    auto& entry = tbl.Entries[i];
        //    if (entry.Name != "QSText") {
        //        continue;
        //    }
        //
        //    QSText m(entry.Data.data(), entry.Data.size());
        //    auto lines = Split(m.Text, "\n");
        //    std::string newString;
        //    bool featured = false;
        //    for (auto& line : lines) {
        //        // drop the featured shop for less clutter
        //        if (line.starts_with("#0C")) {
        //            if (featured) {
        //                break;
        //            }
        //            featured = true;
        //            continue;
        //        }
        //
        //        bool hasLocation = false;
        //        for (size_t i = 3; i < line.size(); ++i) {
        //            if (line[i] == '#') {
        //                break;
        //            }
        //            if (line[i] == ':') {
        //                hasLocation = true;
        //                break;
        //            }
        //        }
        //
        //        bool reachedItem = false;
        //        bool reachedLocation = false;
        //        bool attachedLocationMarker = false;
        //        newString += line.substr(0, 3);
        //        for (size_t i = 3; i < line.size(); ++i) {
        //            if (!reachedItem && line[i] == '#') {
        //                newString += 'I';
        //                reachedItem = true;
        //            }
        //            if (line[i] == ' ') {
        //                newString += line[i];
        //                continue;
        //            }
        //            if (line.substr(i, 3) == "\xe3\x80\x80") {
        //                newString += line.substr(i, 3);
        //                i += 2;
        //                continue;
        //            }
        //            if (!attachedLocationMarker
        //                && (!hasLocation || (hasLocation && reachedLocation))) {
        //                newString += 'I';
        //                attachedLocationMarker = true;
        //            }
        //            if (hasLocation && !reachedLocation && line[i] == ':') {
        //                reachedLocation = true;
        //            }
        //            newString += line[i];
        //        }
        //        newString += "\n";
        //    }
        //    if (!newString.empty()) {
        //        newString.pop_back();
        //    }
        //    m.Text = newString;
        //    entry.Data = m.ToBinary();
        //}

        // format the story log in a consistent manner
        for (size_t i = 34; i < tbl.Entries.size(); ++i) {
            auto& entry = tbl.Entries[i];
            if (entry.Name != "QSText") {
                continue;
            }

            QSText m(entry.Data.data(), entry.Data.size());
            auto lines = Split(m.Text, "\n");
            std::string newString;
            for (auto& line : lines) {
                newString += ReformatStoryLine(line, 0);
                newString += "\n";
            }
            if (!newString.empty()) {
                newString.pop_back();
            }
            m.Text = newString;
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
} // namespace SenLib::TX::FileFixesSw::t_main

namespace SenLib::TX::FileFixesSw::t_quest {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "text/dat/t_quest.tbl",
            41258,
            HyoutaUtils::Hash::SHA1FromHexString("69678b0e57fe0a386dcf82c3923c27759ef85ccd"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto& bin = fileSw->GetVectorData();
        SenLib::TX::Tbl tbl(bin.data(), bin.size());
        using namespace HyoutaUtils::TextUtils;

        // these are difficult to determine automatically so just hardcode them, there's only a few
        static constexpr std::array<std::pair<int, int>, 16> authorPrefixCounts = {{
            {37, 22},  // Ch.2; My Precious; Nodoka
            {54, 16},  // Ch.3; Tofu Trader's Trusty Trumpet; Takahiro Suzuki
            {64, 21},  // Ch.4; Poor Unfortunate Soles; Takashi
            {74, 12},  // Ch.4; Shine On, You Crazy Diamond; Narushima Jewelers, Kozue
            {84, 22},  // Ch.5; Do It for the Vine; Fuuka
            {89, 22},  // Ch.5; Ingredients, Dive-Ins and Dives; Mayu
            {102, 18}, // Ch.6; Lights! Camera! ACTION!; Kousaku & Minoru
            {113, 21}, // Ch.6; Tsubasa's Day Off; Sorayama
            {124, 10}, // Ch.7; What the XRC Does in the Shadows; Hekiru, President, Journalism Club
            {131, 20}, // Ch.7; Where in the World is Airi?; Takeuchi
            {141, 18}, // Ch.7; Treasure that Trascends Time; Mitsu Koizumi
            {224, 20}, // After; This Is Halloween; Azusa
            {246, 20}, // After; Pumpkin Spice and Everything Nice; Yanagi
            {266, 18}, // After; Lager Than Life; Hamaguchi
            {293, 16}, // After; Nice Boat!; Iijima
            {308, 19}, // After; Dance, Morimaru, Dance!; Staff
        }};

        // remove a few empty lines so that reading the whole quest description doesn't you require
        // to tab into the description and scroll up
        static constexpr std::array<std::pair<int, int>, 6> trashEmptyLines = {{
            {64, 1},  // Poor Unfortunate Soles
            {113, 1}, // Tsubasa's Day Off
            {124, 2}, // What the XRC Does in the Shadows
            {246, 1}, // Pumpkin Spice and Everything Nice
            {266, 2}, // Lager Than Life
            {293, 1}, // Nice Boat!
        }};

        // entry 110 is missing a separator between the quest description and the log, copy that
        // from elsewhere
        {
            auto& entry = tbl.Entries[110];
            QSText m(entry.Data.data(), entry.Data.size());
            auto& other = tbl.Entries[30];
            QSText m2(other.Data.data(), other.Data.size());
            m.Text = (std::string(Split(m2.Text, "\n").front()) + "\n" + m.Text);
            entry.Data = m.ToBinary();
        }

        // entry 127 has a typo and is missing the period at the end
        // "Aat least we managed to survive the interview"
        {
            auto& entry = tbl.Entries[127];
            QSText m(entry.Data.data(), entry.Data.size());
            m.Text = Remove(m.Text, 187, 1) + ".";
            entry.Data = m.ToBinary();
        }

        // format the quest log in a consistent manner, matching the story log
        for (size_t i = 0; i < tbl.Entries.size(); ++i) {
            auto& entry = tbl.Entries[i];
            if (entry.Name != "QSText") {
                continue;
            }

            QSText m(entry.Data.data(), entry.Data.size());
            auto lines = Split(m.Text, "\n");
            const size_t authorPrefix = [&]() -> size_t {
                for (const auto& a : authorPrefixCounts) {
                    if (static_cast<size_t>(a.first) == i) {
                        return static_cast<size_t>(a.second);
                    }
                }
                return 0u;
            }();
            const size_t emptyLines = [&]() -> size_t {
                for (const auto& a : trashEmptyLines) {
                    if (static_cast<size_t>(a.first) == i) {
                        return static_cast<size_t>(a.second);
                    }
                }
                return 0u;
            }();

            std::vector<std::string> newLines;
            for (auto& line : lines) {
                newLines.push_back(ReformatStoryLine(line, authorPrefix));
            }

            for (size_t i = 0; i < emptyLines; ++i) {
                for (size_t j = newLines.size(); j > 0; --j) {
                    auto& l = newLines[j - 1];
                    if (l.empty()) {
                        newLines.erase(newLines.begin() + (j - 1));
                        break;
                    }
                }
            }

            std::string newString;
            for (auto& line : newLines) {
                newString += line;
                newString += "\n";
            }
            if (!newString.empty()) {
                newString.pop_back();
            }
            m.Text = newString;
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
} // namespace SenLib::TX::FileFixesSw::t_quest
