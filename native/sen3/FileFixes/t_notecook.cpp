#include <cstdint>
#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen3/file_fixes.h"
#include "sen3/tbl.h"
#include "sha1.h"
#include "util/text.h"

namespace SenLib::Sen3::FileFixes::t_notecook {
std::string_view GetDescription() {
    return "Fix incorrect recipe descriptions.";
}

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
    const auto& area_data = tbl_itemhelp_en.Entries[area < 1000 ? 182 : 184].Data;
    std::string area_string = CompHelpData(area_data.data(), area_data.size()).str;
    const auto& power_data =
        tbl_itemhelp_en.Entries[power <= 130 ? 204 : (power <= 150 ? 206 : 208)].Data;
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
            SenPatcher::SHA1FromHexString("9155e5158bb3e333654dd761072a04e0a8be6728"));
        auto file_text_en = getCheckedFile(
            "data/text/dat_en/t_text.tbl",
            32728,
            SenPatcher::SHA1FromHexString("a2720e94f597640decd1d978b6b8f731147578a6"));
        auto file_item_en = getCheckedFile(
            "data/text/dat_en/t_item_en.tbl",
            311891,
            SenPatcher::SHA1FromHexString("5deee9b833b2bb93b0a326f586943f3d2e2424b9"));
        auto file_itemhelp_en = getCheckedFile(
            "data/text/dat_en/t_itemhelp.tbl",
            9902,
            SenPatcher::SHA1FromHexString("cb9135407b8264ac813e921329374a844f55036b"));
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
        //         m.item1line1 = "";
        //         m.item1line2 = "";
        //         m.item2line1 = "";
        //         m.item2line2 = "";
        //         m.item3line1 = "";
        //         m.item3line2 = "";
        //         m.item4line1 = "";
        //         m.item4line2 = "";
        //         e.Data = m.ToBinary();
        //     }
        // }

        // Piled Onion Rings
        {
            auto& e = tbl_en.Entries[2];
            CookData m(e.Data.data(), e.Data.size());
            size_t slashpos = m.item4line2.find_first_of('/');
            if (slashpos != std::string::npos) {
                m.item4line1 = m.item4line1 + "/"
                               + std::string(std::string_view(m.item4line2.begin(),
                                                              m.item4line2.begin() + slashpos)
                                                 .substr(10));
                auto i4l2 = " "
                            + std::string(std::string_view(m.item4line2.begin() + slashpos + 1,
                                                           m.item4line2.end()));
                m.item4line2 = std::move(i4l2);
                auto item3 = FindItem(m.item3, tbl_item_en);
                if (item3) {
                    auto effect = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                    if (effect) {
                        m.item3line1 =
                            " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                        m.item3line2 =
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
            m.item1line1 = Replace(m.item1line1, "EP", "CP");
            m.item2line1 = Replace(m.item2line1, "EP", "CP");
            m.item4line1 = Replace(m.item4line1, "EP", "CP");
            e.Data = m.ToBinary();
        }

        // Juicy Ham Sandwich
        {
            auto& e = tbl_en.Entries[6];
            CookData m(e.Data.data(), e.Data.size());
            auto item3 = FindItem(m.item3, tbl_item_en);
            if (item3) {
                auto effect = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                if (effect) {
                    m.item3line1 =
                        " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                    m.item3line2 =
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
            m.item1line1 = Replace(m.item1line1, "EP", "CP");
            m.item2line1 = Replace(m.item2line1, "EP", "CP");
            m.item4line1 = Replace(m.item4line1, "EP", "CP");
            e.Data = m.ToBinary();
        }

        // Dragon Fried Rice
        {
            auto& e = tbl_en.Entries[8];
            CookData m(e.Data.data(), e.Data.size());
            size_t slashpos = m.item1line1.find_first_of('/');
            if (slashpos != std::string::npos) {
                auto i1l1 = std::string(
                    std::string_view(m.item1line1.begin(), m.item1line1.begin() + slashpos));
                auto i1l2 = " "
                            + std::string(std::string_view(m.item1line1.begin() + slashpos + 1,
                                                           m.item1line1.end()))
                            + "/" + m.item1line2.substr(1);
                m.item1line1 = std::move(i1l1);
                m.item1line2 = std::move(i1l2);
                auto item3 = FindItem(m.item3, tbl_item_en);
                if (item3) {
                    auto effect = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                    if (effect) {
                        m.item3line1 =
                            " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                        m.item3line2 =
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
            auto item3 = FindItem(m.item3, tbl_item_en);
            if (item3) {
                auto effect = FindItemHelp(item3->effect2[0], tbl_itemhelp_en);
                if (effect) {
                    m.item3line1 = " ";
                    m.item3line2 = " " + effect->str;
                    e.Data = m.ToBinary();
                }
            }
        }

        // Thick Hamburger Steak
        {
            auto& e = tbl_en.Entries[11];
            CookData m(e.Data.data(), e.Data.size());
            auto item3 = FindItem(m.item3, tbl_item_en);
            if (item3) {
                auto effect = FindItemHelp(item3->effect2[0], tbl_itemhelp_en);
                if (effect) {
                    m.item3line1 = " ";
                    m.item3line2 = " " + effect->str;
                    e.Data = m.ToBinary();
                }
            }
        }

        // Colorful Bouillabaisse
        {
            auto& e = tbl_en.Entries[12];
            CookData m(e.Data.data(), e.Data.size());
            m.item1line1 = Replace(m.item1line1, "EP", "CP");
            m.item2line1 = Replace(m.item2line1, "EP", "CP");
            m.item3line2 = Replace(m.item3line1, "EP", "CP");
            m.item3line1 = " ";
            e.Data = m.ToBinary();
        }

        // Fisherman's Paella
        {
            auto& e = tbl_en.Entries[13];
            CookData m(e.Data.data(), e.Data.size());
            m.item4line1 = Replace(m.item4line1, "EP", "CP");
            e.Data = m.ToBinary();
        }

        // Heavy Meat Pie
        {
            auto& e = tbl_en.Entries[15];
            CookData m(e.Data.data(), e.Data.size());
            auto item3 = FindItem(m.item3, tbl_item_en);
            if (item3) {
                auto effect = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                if (effect) {
                    m.item3line1 =
                        " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                    m.item3line2 =
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
            auto item3 = FindItem(m.item3, tbl_item_en);
            if (item3) {
                auto effect0 = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                auto effect1 = FindItemHelp(item3->effect4[0], tbl_itemhelp_en);
                if (effect0 && effect1) {
                    m.item3line1 =
                        " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                    m.item3line2 =
                        " "
                        + Replace(Replace(effect0->str, "%d", UInt32ToString(item3->effect3[1])),
                                  "%%",
                                  "%")
                        + "/"
                        + Replace(Replace(effect1->str, "%d", UInt32ToString(item3->effect4[1])),
                                  "%%",
                                  "%");
                    e.Data = m.ToBinary();
                }
            }
        }

        // Tomato Curry
        {
            auto& e = tbl_en.Entries[18];
            CookData m(e.Data.data(), e.Data.size());
            m.item1line1 = Replace(m.item1line1, "EP", "CP");
            m.item2line1 = Replace(m.item2line1, "EP", "CP");
            m.item3line1 = Replace(m.item3line1, "EP", "CP");
            e.Data = m.ToBinary();
        }

        // White Velvet Shortcake
        {
            auto& e = tbl_en.Entries[19];
            CookData m(e.Data.data(), e.Data.size());
            auto item4 = FindItem(m.item4, tbl_item_en);
            if (item4) {
                auto effect = FindItemHelp(item4->effect2[0], tbl_itemhelp_en);
                if (effect) {
                    m.item4line1 = " ";
                    m.item4line2 = " " + effect->str;
                    e.Data = m.ToBinary();
                }
            }
        }

        // Croquette Burger
        {
            auto& e = tbl_en.Entries[20];
            CookData m(e.Data.data(), e.Data.size());
            auto item3 = FindItem(m.item3, tbl_item_en);
            if (item3) {
                auto effect0 = FindItemHelp(item3->effect3[0], tbl_itemhelp_en);
                auto effect1 = FindItemHelp(item3->effect4[0], tbl_itemhelp_en);
                if (effect0 && effect1) {
                    m.item3line1 =
                        " " + GenerateAttackItemLine1(*item3, tbl_text_en, tbl_itemhelp_en);
                    m.item3line2 =
                        " "
                        + Replace(Replace(effect0->str, "%d", UInt32ToString(item3->effect3[1])),
                                  "%%",
                                  "%")
                        + "/"
                        + Replace(Replace(effect1->str, "%d", UInt32ToString(item3->effect4[1])),
                                  "%%",
                                  "%");
                    e.Data = m.ToBinary();
                }
            }
        }

        // Hearty Kebab
        {
            auto& e = tbl_en.Entries[22];
            CookData m(e.Data.data(), e.Data.size());
            m.item1line1 = Replace(m.item1line1, "EP", "CP");
            m.item2line1 = Replace(m.item2line1, "EP", "CP");
            m.item3line1 = Replace(m.item3line1, "EP", "CP");
            m.item4line1 = Replace(m.item4line1, "EP", "CP");
            e.Data = m.ToBinary();
        }

        std::vector<char> result_en_vec;
        MemoryStream result_en(result_en_vec);
        tbl_en.WriteToStream(result_en, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(
            std::move(result_en_vec), file_en->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t_notecook
