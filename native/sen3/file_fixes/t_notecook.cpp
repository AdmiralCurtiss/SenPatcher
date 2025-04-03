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
__declspec(dllexport) char SenPatcherFix_0_notecook[] = "Fix incorrect recipe descriptions.";
}

namespace SenLib::Sen3::FileFixes::t_notecook {
static std::optional<ItemData> FindItem(uint16_t itemIdx, const Tbl& tbl) {
    // not very efficient but it's not called often enough for this to matter...
    for (size_t i = 0; i < tbl.Entries.size(); ++i) {
        if (tbl.Entries[i].Name == "item") {
            ItemData item(tbl.Entries[i].Data.data(), tbl.Entries[i].Data.size());
            if (item.idx == itemIdx) {
                return item;
            }
        }
    }
    return std::nullopt;
}

static std::optional<ItemHelpData> FindItemHelp(uint16_t helpIdx, const Tbl& tbl) {
    // not very efficient but it's not called often enough for this to matter...
    for (size_t i = 0; i < tbl.Entries.size(); ++i) {
        if (tbl.Entries[i].Name == "ItemHelpData") {
            ItemHelpData item(tbl.Entries[i].Data.data(), tbl.Entries[i].Data.size());
            if (item.idx == helpIdx) {
                return item;
            }
        }
    }
    return std::nullopt;
}

static std::string GenerateAttackItemLine1(ItemData item, Tbl tbl_text_en, Tbl tbl_itemhelp_en) {
    // tbqh I'm not sure how all these connections work and if the fields are right, but this works
    // out for the given items anyway
    uint16_t area = item.effect1[0];
    uint16_t power = item.effect2[1];
    const auto& area_data = tbl_itemhelp_en.Entries[area < 1000u ? 182u : 184u].Data;
    std::string area_string = CompHelpData(area_data.data(), area_data.size()).str;
    const auto& power_data =
        tbl_itemhelp_en.Entries[power <= 130u ? 204u : (power <= 150u ? 206u : 208u)].Data;
    std::string power_string = CompHelpData(power_data.data(), power_data.size()).str;
    const auto& a_data = tbl_text_en.Entries[205].Data;
    std::string a = TextTableData(a_data.data(), a_data.size()).str.substr(0, 7);
    const auto& b_data = tbl_text_en.Entries[220].Data;
    std::string b = HyoutaUtils::TextUtils::Replace(
        TextTableData(b_data.data(), b_data.size()).str, "%s", area_string);
    return a + "(Power " + power_string + ") - " + b;
}


bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    using namespace HyoutaUtils::TextUtils;

    try {
        auto file_en = getCheckedFile(
            "data/text/dat_en/t_notecook.tbl",
            6778,
            HyoutaUtils::Hash::SHA1FromHexString("9155e5158bb3e333654dd761072a04e0a8be6728"));
        auto file_text_en = getCheckedFile(
            "data/text/dat_en/t_text.tbl",
            32728,
            HyoutaUtils::Hash::SHA1FromHexString("a2720e94f597640decd1d978b6b8f731147578a6"));
        auto file_item_en = getCheckedFile(
            "data/text/dat_en/t_item_en.tbl",
            311891,
            HyoutaUtils::Hash::SHA1FromHexString("5deee9b833b2bb93b0a326f586943f3d2e2424b9"));
        auto file_itemhelp_en = getCheckedFile(
            "data/text/dat_en/t_itemhelp.tbl",
            9902,
            HyoutaUtils::Hash::SHA1FromHexString("cb9135407b8264ac813e921329374a844f55036b"));
        if (!file_en || !file_text_en || !file_item_en || !file_itemhelp_en) {
            return false;
        }

        Tbl tbl_en(file_en->Data.data(),
                   file_en->Data.size(),
                   HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        Tbl tbl_text_en(file_text_en->Data.data(),
                        file_text_en->Data.size(),
                        HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        Tbl tbl_item_en(file_item_en->Data.data(),
                        file_item_en->Data.size(),
                        HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        Tbl tbl_itemhelp_en(file_itemhelp_en->Data.data(),
                            file_itemhelp_en->Data.size(),
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
        //     auto& e = tbl_en.Entries[i];
        //     if (e.Name == "QSCook") {
        //         CookData m(e.Data.data(), e.Data.size());
        //         m.items[0].lines[0] = "";
        //         m.items[0].lines[1] = "";
        //         m.items[1].lines[0] = "";
        //         m.items[1].lines[1] = "";
        //         m.items[2].lines[0] = "";
        //         m.items[2].lines[1] = "";
        //         m.items[3].lines[0] = "";
        //         m.items[3].lines[1] = "";
        //         e.Data = m.ToBinary();
        //     }
        // }

        // Piled Onion Rings
        {
            auto& e = tbl_en.Entries[2];
            CookData m(e.Data.data(), e.Data.size());
            size_t slashpos = m.items[3].lines[1].find_first_of('/');
            if (slashpos != std::string::npos) {
                m.items[3].lines[0] =
                    m.items[3].lines[0] + "/"
                    + std::string(std::string_view(m.items[3].lines[1].begin(),
                                                   m.items[3].lines[1].begin() + slashpos)
                                      .substr(10));
                std::string i4l2 =
                    " "
                    + std::string(std::string_view(m.items[3].lines[1].begin() + slashpos + 1,
                                                   m.items[3].lines[1].end()));
                m.items[3].lines[1] = std::move(i4l2);
                auto item3 = FindItem(m.items[2].id, tbl_item_en);
                if (item3) {
                    auto effect = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                    if (effect) {
                        m.items[2].lines[0] =
                            " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                        m.items[2].lines[1] =
                            " "
                            + Replace(Replace(effect->str, "%d", UInt32ToString(item3->effect3[1])),
                                      "%%",
                                      "%");
                        e.Data = m.ToBinary();
                    }
                }
            }
        }

        // Southern Punch
        {
            auto& e = tbl_en.Entries[4];
            CookData m(e.Data.data(), e.Data.size());
            m.items[0].lines[0] = Replace(m.items[0].lines[0], "EP", "CP");
            m.items[1].lines[0] = Replace(m.items[1].lines[0], "EP", "CP");
            m.items[3].lines[0] = Replace(m.items[3].lines[0], "EP", "CP");
            e.Data = m.ToBinary();
        }

        // Juicy Ham Sandwich
        {
            auto& e = tbl_en.Entries[6];
            CookData m(e.Data.data(), e.Data.size());
            auto item3 = FindItem(m.items[2].id, tbl_item_en);
            if (item3) {
                auto effect = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                if (effect) {
                    m.items[2].lines[0] =
                        " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                    m.items[2].lines[1] =
                        " "
                        + Replace(Replace(effect->str, "%d", UInt32ToString(item3->effect3[1])),
                                  "%%",
                                  "%");
                    e.Data = m.ToBinary();
                }
            }
        }

        // Fresh Tomato Noodles
        {
            auto& e = tbl_en.Entries[7];
            CookData m(e.Data.data(), e.Data.size());
            m.items[0].lines[0] = Replace(m.items[0].lines[0], "EP", "CP");
            m.items[1].lines[0] = Replace(m.items[1].lines[0], "EP", "CP");
            m.items[3].lines[0] = Replace(m.items[3].lines[0], "EP", "CP");
            e.Data = m.ToBinary();
        }

        // Dragon Fried Rice
        {
            auto& e = tbl_en.Entries[8];
            CookData m(e.Data.data(), e.Data.size());
            size_t slashpos = m.items[0].lines[0].find_first_of('/');
            if (slashpos != std::string::npos) {
                auto i1l1 = std::string(std::string_view(m.items[0].lines[0].begin(),
                                                         m.items[0].lines[0].begin() + slashpos));
                auto i1l2 =
                    " "
                    + std::string(std::string_view(m.items[0].lines[0].begin() + slashpos + 1,
                                                   m.items[0].lines[0].end()))
                    + "/" + m.items[0].lines[1].substr(1);
                m.items[0].lines[0] = std::move(i1l1);
                m.items[0].lines[1] = std::move(i1l2);
                auto item3 = FindItem(m.items[2].id, tbl_item_en);
                if (item3) {
                    auto effect = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                    if (effect) {
                        m.items[2].lines[0] =
                            " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                        m.items[2].lines[1] =
                            " "
                            + Replace(Replace(effect->str, "%d", UInt32ToString(item3->effect3[1])),
                                      "%%",
                                      "%");
                        e.Data = m.ToBinary();
                    }
                }
            }
        }

        // Honey Bagel
        {
            auto& e = tbl_en.Entries[10];
            CookData m(e.Data.data(), e.Data.size());
            auto item3 = FindItem(m.items[2].id, tbl_item_en);
            if (item3) {
                auto effect = FindItemHelp(item3->effect2[0], tbl_itemhelp_en);
                if (effect) {
                    m.items[2].lines[0] = " ";
                    m.items[2].lines[1] = " " + effect->str;
                    e.Data = m.ToBinary();
                }
            }
        }

        // Thick Hamburger Steak
        {
            auto& e = tbl_en.Entries[11];
            CookData m(e.Data.data(), e.Data.size());
            auto item3 = FindItem(m.items[2].id, tbl_item_en);
            if (item3) {
                auto effect = FindItemHelp(item3->effect2[0], tbl_itemhelp_en);
                if (effect) {
                    m.items[2].lines[0] = " ";
                    m.items[2].lines[1] = " " + effect->str;
                    e.Data = m.ToBinary();
                }
            }
        }

        // Colorful Bouillabaisse
        {
            auto& e = tbl_en.Entries[12];
            CookData m(e.Data.data(), e.Data.size());
            m.items[0].lines[0] = Replace(m.items[0].lines[0], "EP", "CP");
            m.items[1].lines[0] = Replace(m.items[1].lines[0], "EP", "CP");
            m.items[2].lines[1] = Replace(m.items[2].lines[0], "EP", "CP");
            m.items[2].lines[0] = " ";
            e.Data = m.ToBinary();
        }

        // Fisherman's Paella
        {
            auto& e = tbl_en.Entries[13];
            CookData m(e.Data.data(), e.Data.size());
            m.items[3].lines[0] = Replace(m.items[3].lines[0], "EP", "CP");
            e.Data = m.ToBinary();
        }

        // Heavy Meat Pie
        {
            auto& e = tbl_en.Entries[15];
            CookData m(e.Data.data(), e.Data.size());
            auto item3 = FindItem(m.items[2].id, tbl_item_en);
            if (item3) {
                auto effect = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                if (effect) {
                    m.items[2].lines[0] =
                        " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                    m.items[2].lines[1] =
                        " "
                        + Replace(Replace(effect->str, "%d", UInt32ToString(item3->effect3[1])),
                                  "%%",
                                  "%");
                    tbl_en.Entries[15].Data = m.ToBinary();
                }
            }
        }

        // Aquamarine Ice Cream
        {
            auto& e = tbl_en.Entries[16];
            CookData m(e.Data.data(), e.Data.size());
            auto item3 = FindItem(m.items[2].id, tbl_item_en);
            if (item3) {
                auto effect0 = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                auto effect1 = FindItemHelp(item3->effect4[0], tbl_itemhelp_en);
                if (effect0 && effect1) {
                    m.items[2].lines[0] =
                        " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                    if (item3->effect3[1] == item3->effect4[1]) {
                        m.items[2].lines[1] =
                            " " + Replace(effect0->str, " (%d%%)", "") + "/"
                            + Replace(
                                Replace(effect1->str, "%d", UInt32ToString(item3->effect4[1])),
                                "%%",
                                "%");
                    } else {
                        m.items[2].lines[1] =
                            " "
                            + Replace(
                                Replace(effect0->str, "%d", UInt32ToString(item3->effect3[1])),
                                "%%",
                                "%")
                            + "/"
                            + Replace(
                                Replace(effect1->str, "%d", UInt32ToString(item3->effect4[1])),
                                "%%",
                                "%");
                    }
                    e.Data = m.ToBinary();
                }
            }
        }

        // Tomato Curry
        {
            auto& e = tbl_en.Entries[18];
            CookData m(e.Data.data(), e.Data.size());
            m.items[0].lines[0] = Replace(m.items[0].lines[0], "EP", "CP");
            m.items[1].lines[0] = Replace(m.items[1].lines[0], "EP", "CP");
            m.items[2].lines[0] = Replace(m.items[2].lines[0], "EP", "CP");
            e.Data = m.ToBinary();
        }

        // White Velvet Shortcake
        {
            auto& e = tbl_en.Entries[19];
            CookData m(e.Data.data(), e.Data.size());
            auto item4 = FindItem(m.items[3].id, tbl_item_en);
            if (item4) {
                auto effect = FindItemHelp(item4->effect2[0], tbl_itemhelp_en);
                if (effect) {
                    m.items[3].lines[0] = " ";
                    m.items[3].lines[1] = " " + effect->str;
                    e.Data = m.ToBinary();
                }
            }
        }

        // Croquette Burger
        {
            auto& e = tbl_en.Entries[20];
            CookData m(e.Data.data(), e.Data.size());
            auto item3 = FindItem(m.items[2].id, tbl_item_en);
            if (item3) {
                auto effect0 = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                auto effect1 = FindItemHelp(item3->effect4[0], tbl_itemhelp_en);
                if (effect0 && effect1) {
                    m.items[2].lines[0] =
                        " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                    if (item3->effect3[1] == item3->effect4[1]) {
                        m.items[2].lines[1] =
                            " " + Replace(effect0->str, " (%d%%)", "") + "/"
                            + Replace(
                                Replace(effect1->str, "%d", UInt32ToString(item3->effect4[1])),
                                "%%",
                                "%");
                    } else {
                        m.items[2].lines[1] =
                            " "
                            + Replace(
                                Replace(effect0->str, "%d", UInt32ToString(item3->effect3[1])),
                                "%%",
                                "%")
                            + "/"
                            + Replace(
                                Replace(effect1->str, "%d", UInt32ToString(item3->effect4[1])),
                                "%%",
                                "%");
                    }
                    e.Data = m.ToBinary();
                }
            }
        }

        // Hearty Kebab
        {
            auto& e = tbl_en.Entries[22];
            CookData m(e.Data.data(), e.Data.size());
            m.items[0].lines[0] = Replace(m.items[0].lines[0], "EP", "CP");
            m.items[1].lines[0] = Replace(m.items[1].lines[0], "EP", "CP");
            m.items[2].lines[0] = Replace(m.items[2].lines[0], "EP", "CP");
            m.items[3].lines[0] = Replace(m.items[3].lines[0], "EP", "CP");
            e.Data = m.ToBinary();
        }

        // "(X turns)" -> "for X turns"
        for (size_t i = 0; i < tbl_en.Entries.size(); ++i) {
            auto& e = tbl_en.Entries[i];
            const auto apply = [&](CookData& m) -> bool {
                bool modified = false;
                for (auto& item : m.items) {
                    for (auto& line : item.lines) {
                        size_t endpos = std::string::npos;
                        while (true) {
                            auto nextCloseParens = line.find_last_of(')', endpos);
                            if (nextCloseParens == std::string::npos) {
                                break;
                            }
                            auto nextOpenParens = line.find_last_of('(', nextCloseParens);
                            if (nextOpenParens == std::string::npos) {
                                break;
                            }

                            auto parensContent = std::string_view(line).substr(
                                nextOpenParens + 1, nextCloseParens - (nextOpenParens + 1));
                            if (!parensContent.empty() && parensContent[0] >= '0'
                                && parensContent[0] <= '9'
                                && (parensContent.ends_with("turn")
                                    || parensContent.ends_with("turns"))) {
                                std::string f = "for ";
                                f += parensContent;
                                line = HyoutaUtils::TextUtils::ReplaceSubstring(
                                    line,
                                    nextOpenParens,
                                    (nextCloseParens + 1) - nextOpenParens,
                                    f);
                                modified = true;
                            }

                            endpos = nextOpenParens;
                        }
                    }
                }
                return modified;
            };
            if (e.Name == "QSCook") {
                CookData m(e.Data.data(), e.Data.size());
                if (apply(m)) {
                    e.Data = m.ToBinary();
                }
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
} // namespace SenLib::Sen3::FileFixes::t_notecook
