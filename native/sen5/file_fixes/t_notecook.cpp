#include <string>
#include <string_view>
#include <utility>
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
            m.items[1].lines[1] = HyoutaUtils::TextUtils::Remove(m.items[1].lines[1], 12, 1);
            e.Data = m.ToBinary();
        }
        {
            auto& e = tbl_en.Entries[4];
            CookData m(e.Data.data(), e.Data.size());
            m.items[1].lines[0] = HyoutaUtils::TextUtils::Remove(m.items[1].lines[0], 21, 1);
            m.items[1].lines[0] = HyoutaUtils::TextUtils::Remove(m.items[1].lines[0], 19, 1);
            m.items[1].lines[1] = HyoutaUtils::TextUtils::Remove(m.items[1].lines[1], 12, 1);
            e.Data = m.ToBinary();
        }

        // Tri-Color Onigiri: Uses a dot where all other descriptions use a slash.
        {
            auto& e = tbl_en.Entries[3];
            CookData m(e.Data.data(), e.Data.size());
            m.items[1].lines[0] =
                HyoutaUtils::TextUtils::ReplaceSubstring(m.items[1].lines[0], 16, 3, "/");
            e.Data = m.ToBinary();
        }

        // All the attack items are missing their Power rating.
        for (const auto& p : {std::pair<int, const char*>(2, "B"),
                              std::pair<int, const char*>(6, "B+"),
                              std::pair<int, const char*>(14, "A"),
                              std::pair<int, const char*>(22, "A+"),
                              std::pair<int, const char*>(27, "S")}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(p.first)];
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                auto& item = m.items[2];
                auto& line = item.lines[0];
                std::string tmp;
                tmp += "Attack (Power ";
                tmp += p.second;
                tmp += ")-";
                line = tmp + line;
                e.Data = m.ToBinary();
            }
        }

        // Several items have "Restores CP+XX" which is an inconsistent phrasing.
        for (int idx : {10, 27}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                for (auto& item : m.items) {
                    auto& line = item.lines[1];
                    auto cp = line.find("CP");
                    if (cp != std::string::npos && cp >= 9) {
                        line = HyoutaUtils::TextUtils::Remove(line, cp - 9, 9);
                    }
                }
                e.Data = m.ToBinary();
            }
        }

        // A few food items have "EP+XX" for recovery which is an inconsistent phrasing.
        for (int idx : {7, 22, 30}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                for (auto& item : m.items) {
                    for (auto& line : item.lines) {
                        auto ep = line.find("EP+");
                        if (ep != std::string::npos) {
                            line = HyoutaUtils::TextUtils::Remove(line, ep, 3);
                            line = HyoutaUtils::TextUtils::Insert(line, ep + 3, " EP");
                            e.Data = m.ToBinary();
                        }
                    }
                }
                e.Data = m.ToBinary();
            }
        }

        // Like crafts we have several instances of uppercase "Ailments" where it usually is not
        // capitalized.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                for (auto& item : m.items) {
                    for (auto& line : item.lines) {
                        auto pos = line.find("Ailments");
                        if (pos != std::string::npos) {
                            line = HyoutaUtils::TextUtils::ReplaceSubstring(line, pos, 1, "a");
                        }
                    }
                }
                e.Data = m.ToBinary();
            }
        }

        // Devil's Food Cornet: This has a somewhat oddly combined "DEF↓(S)/Insight for 2 turns",
        // split that.
        {
            auto& e = tbl_en.Entries[12];
            CookData m(e.Data.data(), e.Data.size());
            auto& line = m.items[1].lines[1];
            std::string turns = line.substr(line.size() - 12);
            line = HyoutaUtils::TextUtils::ReplaceSubstring(line, 9, 1, turns + '-');
            e.Data = m.ToBinary();
        }

        // Add a space to the "-", it looks terrible without it.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                for (auto& item : m.items) {
                    for (auto& line : item.lines) {
                        line = HyoutaUtils::TextUtils::Replace(line, "-", " - ");
                    }
                }
                e.Data = m.ToBinary();
            }
        }

        // replace the separator dot between STR/DEF etc. with a slightly smaller one that's used by
        // the autogenerator, which looks a bit better and is consistent with the autogenerator.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                for (auto& item : m.items) {
                    for (auto& line : item.lines) {
                        line =
                            HyoutaUtils::TextUtils::Replace(line, "\xE3\x83\xBB", "\xEF\xBD\xA5");
                    }
                }
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
