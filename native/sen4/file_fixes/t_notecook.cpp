#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen4/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_0_notecook[] = "Improve formatting of cooking effects.";
}

namespace SenLib::Sen4::FileFixes::t_notecook {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file_en = getCheckedFile(
            "data/text/dat_en/t_notecook.tbl",
            7638,
            HyoutaUtils::Hash::SHA1FromHexString("c755b53e8ba8618e9e55d4e7c83cccb7e7298fbb"));
        if (!file_en) {
            return false;
        }

        auto& bin = file_en->Data;

        Tbl tbl_en(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // A few items have "CP +XX", that's inconsistent as usually that space isn't there.
        for (int idx : {8, 14, 23}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(idx)];
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                for (auto& item : m.items) {
                    auto& line = item.lines[1];
                    auto cp = line.find("CP +");
                    if (cp != std::string::npos) {
                        line = HyoutaUtils::TextUtils::Remove(line, cp + 2, 1);
                    }
                }
                e.Data = m.ToBinary();
            }
        }

        // All the attack items are missing their Area. They also list the Power as 'Class'.
        for (const auto& p : {std::pair<int, const char*>(3, "S"),
                              std::pair<int, const char*>(7, "S"),
                              std::pair<int, const char*>(12, "S"),
                              std::pair<int, const char*>(16, "M"),
                              std::pair<int, const char*>(19, "M"),
                              std::pair<int, const char*>(21, "M")}) {
            auto& e = tbl_en.Entries[static_cast<size_t>(p.first)];
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                auto& item = m.items[2];
                auto& line = item.lines[0];
                auto start = line.find('-');
                auto end = line.rfind(' ');
                if (start != std::string::npos && end != std::string::npos && start < end) {
                    line = HyoutaUtils::TextUtils::ReplaceSubstring(line, start, 7, "(Power");
                    line += ")";
                }
                line += " - Area ";
                line += p.second;
                line += " (Set)";
                e.Data = m.ToBinary();
            }
        }

        // Practically every stat up effect has extra spaces around the up arrow, fix that.
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                for (auto& item : m.items) {
                    for (auto& line : item.lines) {
                        auto up = line.find("\xE2\x86\x91");
                        if (up != std::string::npos) {
                            size_t right = up + 2;
                            while (right < (line.size() - 1) && line[right + 1] == ' ') {
                                ++right;
                            }
                            if (right != (up + 2)) {
                                line =
                                    HyoutaUtils::TextUtils::Remove(line, up + 3, right - (up + 2));
                            }
                            size_t left = up;
                            while (left > 0 && line[left - 1] == ' ') {
                                --left;
                            }
                            if (left != up) {
                                line = HyoutaUtils::TextUtils::Remove(line, left, up - left);
                            }
                        }
                    }
                }
                e.Data = m.ToBinary();
            }
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
} // namespace SenLib::Sen4::FileFixes::t_notecook
