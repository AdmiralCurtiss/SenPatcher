#include <string_view>
#include <unordered_map>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen1/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

#include "text_dat_us_t_item_tbl_t_magic_tbl.h"

namespace SenLib::Sen1::FileFixes::text_dat_us_t_item_tbl_t_magic_tbl {
std::string FixHpEpCpSpacing(std::string desc, size_t start) {
    for (size_t i = start; i < desc.size(); ++i) {
        if ((desc[i - 2] >= '0' && desc[i - 2] <= '9')
            && (desc[i - 1] == 'H' || desc[i - 1] == 'E' || desc[i - 1] == 'C') && desc[i] == 'P') {
            desc.insert(desc.begin() + i - 1, ' ');
            return FixHpEpCpSpacing(std::move(desc), i + 1);
        }
    }
    return desc;
}

static const char* GetMagicClass(long value) {
    const char* c = "D";
    if (value >= 120)
        c = "C";
    if (value >= 135)
        c = "C+";
    if (value >= 170)
        c = "B";
    if (value >= 200)
        c = "B+";
    if (value >= 210)
        c = "A";
    if (value >= 240)
        c = "A+";
    if (value >= 255)
        c = "S";
    if (value >= 295)
        c = "S+";
    if (value >= 315)
        c = "SS";
    if (value >= 345)
        c = "SS+";
    if (value >= 370)
        c = "SSS";
    if (value >= 405)
        c = "SSS+";
    if (value >= 450)
        c = "4S";
    if (value >= 600)
        c = "5S";
    return c;
}

using namespace HyoutaUtils::TextUtils;

static void FixRarenessIdentifierInDescription(ItemData& item) {
    // first identify what's already here
    uint8_t identifiedRarity = 0;
    if (item.Desc.size() >= 8 && item.Desc[5] == '(' && item.Desc[6] == 'R'
        && item.Desc[7] == ')') {
        identifiedRarity = 1;
    } else if (item.Desc.size() >= 9 && item.Desc[5] == '(' && item.Desc[6] == 'S'
               && item.Desc[7] == 'R' && item.Desc[8] == ')') {
        identifiedRarity = 2;
    }

    // if it's different fix it up
    if (identifiedRarity != item.Action2_Rarity) {
        if (identifiedRarity != 0) {
            item.Desc = Remove(item.Desc, 5, identifiedRarity == 1 ? 3 : 4);
        }
        if (item.Action2_Rarity == 1) {
            item.Desc = Insert(item.Desc, 5, "(R)");
        } else if (item.Action2_Rarity == 2) {
            item.Desc = Insert(item.Desc, 5, "(SR)");
        }
    }
}

bool IsSingleArt(const ItemData& item) {
    return item.Action2Value1_Art1 != 0xffff && item.Action1Value1_Art2 == 0xffff
           && item.Action1Value2_Art3 == 0xffff;
}

bool IsStatusQuartz(const ItemData& item) {
    if (item.Action2Value1_Art1 == 0xffff && item.Action1Value1_Art2 == 0xffff
        && item.Action1Value2_Art3 == 0xffff && item.Status_Breaker == 0 && item.Range == 0
        && item.AttackArea == 0 && item.Action2_Rarity == 0 && item.Action1_PassiveEffect == 0
        && item.Action2Value2_ProcChance == 0) {
        int c = 0;
        c += (item.STR != 0 ? 1 : 0);
        c += (item.DEF != 0 ? 1 : 0);
        c += (item.ATS != 0 ? 1 : 0);
        c += (item.ADF != 0 ? 1 : 0);
        c += (item.ACC != 0 ? 1 : 0);
        c += (item.EVA != 0 ? 1 : 0);
        c += (item.SPD != 0 ? 1 : 0);
        c += (item.MOV != 0 ? 1 : 0);
        c += (item.HP != 0 ? 1 : 0);
        c += (item.EP != 0 ? 1 : 0);
        return c == 1;
    }
    return false;
}

bool IsSpecialQuartz(const ItemData& item) {
    if (item.Action2Value1_Art1 == 0xffff && item.Action1Value1_Art2 == 0xffff
        && item.Action1Value2_Art3 == 0xffff && item.Action1_PassiveEffect != 0) {
        return true;
    }
    if (item.Idx == 0x88c || item.Idx == 0x8ec) {
        return true;
    }
    return false;
}

static void PatchItemTbl(Tbl& tbl) {
    // List<ItemData> items = new List<ItemData>();
    // foreach (TblEntry entry in tbl.Entries) {
    //     items.Add(new ItemData(entry.Data));
    // }

    {
        // stray letter
        auto& e = tbl.Entries[743];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = Remove(item.Desc, 8, 1);
        e.Data = item.ToBinary();
    }

    {
        // incorrect plural
        auto& e = tbl.Entries[105];
        ItemData item(e.Data.data(), e.Data.size());
        item.Name = Remove(item.Name, item.Name.size() - 1, 1);
        e.Data = item.ToBinary();
    }

    {
        // missing stat
        auto& e = tbl.Entries[157];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = InsertSubstring(
            item.Desc,
            20,
            ItemData(tbl.Entries[156].Data.data(), tbl.Entries[156].Data.size()).Desc,
            7,
            6);
        e.Data = item.ToBinary();
    }

    {
        // missing equip limitation
        auto& e = tbl.Entries[159];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = InsertSubstring(
            item.Desc,
            14,
            ItemData(tbl.Entries[168].Data.data(), tbl.Entries[168].Data.size()).Desc,
            27,
            11);
        e.Data = item.ToBinary();
    }

    // stat formatting consistency
    {
        auto& e = tbl.Entries[363];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = InsertSubstring(item.Desc, 11, item.Desc, 15, 2);
        e.Data = item.ToBinary();
    }
    for (int idx : {143, 179, 217, 242, 250}) {
        auto& e = tbl.Entries[idx];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = Replace(item.Desc, "\xe3\x80\x90", "[");
        item.Desc = Replace(item.Desc, "\xe3\x80\x91", "]");
        item.Desc = Replace(item.Desc, " ", "/");
        e.Data = item.ToBinary();
    }

    for (int i = 873; i < 953; ++i) {
        // stray spaces at the end of the item name
        int idx = i;
        auto& e = tbl.Entries[idx];
        ItemData item(e.Data.data(), e.Data.size());
        if (item.Name.ends_with(" ")) {
            item.Name = item.Name.substr(0, item.Name.size() - 1);
            tbl.Entries[idx].Data = item.ToBinary();
        }
    }

    // a bunch of 'easy to use' -> 'easy-to-use' and the like
    {
        auto& e = tbl.Entries[127];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 28, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 31, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[128];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 41, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 44, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[144];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 50, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 53, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[147];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 64, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 67, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[150];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 64, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 67, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[151];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 64, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 67, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[185];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 23, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 26, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[192];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 24, 1, " ", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 36, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 39, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[193];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 49, 1, " ", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 61, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 64, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[196];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 49, 1, " ", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 61, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 64, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[199];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 49, 1, " ", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 61, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 64, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[225];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 30, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 33, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[251];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 20, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 23, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[252];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 20, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 23, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[253];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 33, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 36, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[197];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 27, 1, "-", 0, 1);
        item.Desc = ReplaceSubstring(item.Desc, 30, 1, "-", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[132];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 38, 1, " ", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[181];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 24, 1, " ", 0, 1);
        e.Data = item.ToBinary();
    }

    // capitalization consistency in Heat Up/R
    {
        ItemData item21(tbl.Entries[21].Data.data(), tbl.Entries[21].Data.size());
        {
            auto& e = tbl.Entries[609];
            ItemData item(e.Data.data(), e.Data.size());
            item.Desc = ReplaceSubstring(item.Desc, 33, 9, item21.Desc, 7, 9);
            item.Desc = ReplaceSubstring(item.Desc, 57, 9, item21.Desc, 7, 9);
            e.Data = item.ToBinary();
        }
        {
            auto& e = tbl.Entries[624];
            ItemData item(e.Data.data(), e.Data.size());
            item.Desc = ReplaceSubstring(item.Desc, 36, 9, item21.Desc, 7, 9);
            item.Desc = ReplaceSubstring(item.Desc, 68, 9, item21.Desc, 7, 9);
            e.Data = item.ToBinary();
        }
    }

    // capitalization consistency in Saintly Force/R
    {
        auto& e = tbl.Entries[728];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 36, 1, "t", 0, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[740];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 39, 1, "t", 0, 1);
        e.Data = item.ToBinary();
    }

    for (int idx : {608, 609, 623, 624, 931}) {
        auto& e = tbl.Entries[idx];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = FixHpEpCpSpacing(item.Desc);
        e.Data = item.ToBinary();
    }

    // double space in Needle Shoot/R
    {
        auto& e = tbl.Entries[537];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = Remove(item.Desc, 19, 1);
        e.Data = item.ToBinary();
    }
    {
        auto& e = tbl.Entries[551];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = Remove(item.Desc, 22, 1);
        e.Data = item.ToBinary();
    }

    // poorly-sounding description on Nightmare, sync to CS2's
    {
        auto& e = tbl.Entries[688];
        ItemData item(e.Data.data(), e.Data.size());
        item.Desc = InsertSubstring(
            item.Desc,
            38,
            ItemData(tbl.Entries[527].Data.data(), tbl.Entries[527].Data.size()).Desc,
            47,
            8);
        item.Desc = ReplaceSubstring(item.Desc, 46, 17, item.Name, 0, item.Name.size());
        e.Data = item.ToBinary();
    }

    // add turn count to Chrono Break/R
    {
        ItemData item671(tbl.Entries[671].Data.data(), tbl.Entries[671].Data.size());
        {
            auto& e = tbl.Entries[672];
            ItemData item(e.Data.data(), e.Data.size());
            item.Desc = InsertSubstring(item.Desc, 33, item671.Desc, 41, 10);
            e.Data = item.ToBinary();
        }
        {
            auto& e = tbl.Entries[686];
            ItemData item(e.Data.data(), e.Data.size());
            item.Desc = InsertSubstring(item.Desc, 36, item671.Desc, 41, 10);
            e.Data = item.ToBinary();
        }
    }
}

static void PatchMagicTbl(Tbl& tbl) {
    // fix typo in Emma's S-Craft
    {
        auto& entry = tbl.Entries[100];
        std::swap(entry.Data[0x37], entry.Data[0x39]);
    }

    // remove 'temporarily' in Forte & La Forte for consistency with CS2
    {
        auto& e = tbl.Entries[34];
        MagicData magic(e.Data.data(), e.Data.size());
        magic.Desc = ReplaceSubstring(magic.Desc, 40, 13, "G", 0, 1);
        e.Data = magic.ToBinary();
    }
    {
        auto& e = tbl.Entries[35];
        MagicData magic(e.Data.data(), e.Data.size());
        magic.Desc = ReplaceSubstring(magic.Desc, 45, 13, "G", 0, 1);
        e.Data = magic.ToBinary();
    }

    // match formatting in Soul Blur to CS2
    {
        auto& e = tbl.Entries[44];
        MagicData magic(e.Data.data(), e.Data.size());
        magic.Desc = InsertSubstring(magic.Desc, 46, "-", 0, 1);
        magic.Desc = ReplaceSubstring(magic.Desc, 51, 1, " ", 0, 1);
        e.Data = magic.ToBinary();
    }

    for (int idx : {32, 33, 75}) {
        auto& e = tbl.Entries[idx];
        MagicData magic(e.Data.data(), e.Data.size());
        magic.Desc = FixHpEpCpSpacing(magic.Desc);
        e.Data = magic.ToBinary();
    }

    // double space in Needle Shoot
    {
        auto& e = tbl.Entries[7];
        MagicData magic(e.Data.data(), e.Data.size());
        magic.Desc = Remove(magic.Desc, 19, 1);
        e.Data = magic.ToBinary();
    }

    // add turn count to Chrono Break
    {
        auto& e = tbl.Entries[49];
        MagicData magic(e.Data.data(), e.Data.size());
        magic.Desc = InsertSubstring(
            magic.Desc,
            33,
            MagicData(tbl.Entries[48].Data.data(), tbl.Entries[48].Data.size()).Desc,
            41,
            10);
        e.Data = magic.ToBinary();
    }

    // List<MagicData> items = new List<MagicData>();
    // foreach (TblEntry entry in tbl.Entries) {
    //     items.Add(new MagicData(entry.Data));
    // }

    // clang-format off
    //var sb = new System.Text.StringBuilder();
    //foreach (TblEntry entry in tbl.Entries) {
    //    var item = new MagicData(entry.Data);
    //    if (item.Effect1_Type == 0x01 || item.Effect1_Type == 0x02 || item.Effect1_Type == 0x70 || (item.Effect1_Type >= 0xd9 && item.Effect1_Type <= 0xdd) || item.Effect1_Type == 0xdf) {
    //        bool isMagic = item.Effect1_Type == 0x02 || item.Effect1_Type == 0x70 || item.Effect1_Type == 0xda;
    //        sb.AppendFormat("{0} / Power {1} / {2} / Craft Class {3} / Art Class {4} / In-Game Description: {5}\n",
    //            item.Name, item.Effect1_Value1, isMagic ? "Magic" : "Physical", Sen2.FileFixes.text_dat_us_t_magic_tbl.GetPhysicalClass(item.Effect1_Value1),
    //            Sen2.FileFixes.text_dat_us_t_magic_tbl.GetMagicClass(item.Effect1_Value1), item.Desc.Replace("\n", "{n}")
    //        );
    //    } else {
    //        //sb.AppendFormat("{0} / In-Game Description: {1}\n", item.Name, item.Desc.Replace("\n", "{n}"));
    //    }
    //}
    //File.WriteAllText(@"c:\__ed8\__script-compare\magic-classes.txt", sb.ToString(), System.Text.Encoding.UTF8);
    // clang-format on

    for (int i = 7; i < 62; i++) {
        auto& e = tbl.Entries[i];
        MagicData m(e.Data.data(), e.Data.size());
        if (m.Effect1_Type == 0x01 || m.Effect1_Type == 0x02 || m.Effect1_Type == 0x70
            || (m.Effect1_Type >= 0xd9 && m.Effect1_Type <= 0xdd) || m.Effect1_Type == 0xdf) {
            size_t where = m.Desc.find_first_of(']');
            if (where != std::string::npos) {
                m.Desc = Insert(m.Desc, where, GetMagicClass(m.Effect1_Value1));
                m.Desc = Insert(m.Desc, where, " - Class ");
                e.Data = m.ToBinary();
            }
        }
    }
}

static void SyncEffect(Tbl& itemTbl, size_t itemId, Tbl& magicTbl, size_t magicId, bool useMagic) {
    auto& itemEntry = itemTbl.Entries[itemId];
    ItemData item(itemEntry.Data.data(), itemEntry.Data.size());
    auto& magicEntry = magicTbl.Entries[magicId];
    MagicData magic(magicEntry.Data.data(), magicEntry.Data.size());
    size_t itemDescEnd = item.Desc.find_last_of('\n');
    size_t magicDescEnd = magic.Desc.find_last_of('\n');
    if (itemDescEnd == std::string::npos)
        return;
    if (magicDescEnd == std::string::npos)
        return;
    if (useMagic) {
        // copy magic to item
        auto effect = magic.Desc.substr(0, magicDescEnd);
        if (item.Desc.size() < 6)
            return;
        if (item.Desc[5] == '(') {
            size_t thing = 5;
            while (item.Desc[thing] != ')') {
                ++thing;
                if (thing >= item.Desc.size())
                    return;
            }
            auto rarity = item.Desc.substr(5, thing - 4);
            effect = Insert(effect, 5, rarity);
        }
        item.Desc = ReplaceSubstring(item.Desc, 0, itemDescEnd, effect, 0, effect.size());
        itemEntry.Data = item.ToBinary();
    } else {
        // copy item to magic
        auto effect = item.Desc.substr(0, itemDescEnd);
        if (effect.size() < 6)
            return;
        if (effect[5] == '(') {
            size_t thing = 5;
            while (effect[thing] != ')') {
                ++thing;
                if (thing >= effect.size())
                    return;
            }
            effect = Remove(effect, 5, thing - 4);
        }
        magic.Desc = ReplaceSubstring(magic.Desc, 0, magicDescEnd, effect, 0, effect.size());
        magicEntry.Data = magic.ToBinary();
    }
}

static void
    SyncDescription(Tbl& itemTbl, size_t itemId, Tbl& magicTbl, size_t magicId, bool useMagic) {
    auto& itemEntry = itemTbl.Entries[itemId];
    ItemData item(itemEntry.Data.data(), itemEntry.Data.size());
    auto& magicEntry = magicTbl.Entries[magicId];
    MagicData magic(magicEntry.Data.data(), magicEntry.Data.size());
    size_t itemDescStart = item.Desc.find_last_of('\n') + 1;
    size_t magicDescStart = magic.Desc.find_last_of('\n') + 1;
    if (itemDescStart == std::string::npos)
        return;
    if (magicDescStart == std::string::npos)
        return;
    if (item.Desc[itemDescStart] == '(') {
        while (item.Desc[itemDescStart] != ')') {
            ++itemDescStart;
            if (itemDescStart >= item.Desc.size())
                return;
        }
        itemDescStart += 2;
        if (itemDescStart >= item.Desc.size())
            return;
    }
    if (useMagic) {
        item.Desc = ReplaceSubstring(item.Desc,
                                     itemDescStart,
                                     item.Desc.size() - itemDescStart,
                                     magic.Desc,
                                     magicDescStart,
                                     magic.Desc.size() - magicDescStart);
        itemEntry.Data = item.ToBinary();
    } else {
        magic.Desc = ReplaceSubstring(magic.Desc,
                                      magicDescStart,
                                      magic.Desc.size() - magicDescStart,
                                      item.Desc,
                                      itemDescStart,
                                      item.Desc.size() - itemDescStart);
        magicEntry.Data = magic.ToBinary();
    }
}

static void SyncItemMagicTbl(Tbl& item_tbl, Tbl& magic_tbl) {
    {
        std::unordered_map<uint16_t, MagicData> magic;
        for (auto& e : magic_tbl.Entries) {
            MagicData m(e.Data.data(), e.Data.size());
            magic.emplace(m.Idx, std::move(m));
        }


        for (TblEntry& entry : item_tbl.Entries) {
            ItemData item(entry.Data.data(), entry.Data.size());
            if (item.ItemType == 0xaa) {
                const bool isSingleArt = [&]() {
                    if (!IsSingleArt(item))
                        return false;
                    const auto it = magic.find(item.Action2Value1_Art1);
                    return it != magic.end() && it->second.Name == item.Name;
                }();

                // series consistency: add R to rare quartzes
                if (isSingleArt && item.Action2_Rarity == 1) {
                    item.Name += " R";
                }

                // clang-format off
                //if (isSingleArt) {
                //    string itemdesc0 = item.Desc.Split('\n')[0].Replace("(R)", "").Replace("(SR)", "");
                //    string itemdesc1 = item.Desc.Split('\n')[1];
                //    if (itemdesc1.StartsWith("(")) {
                //        itemdesc1 = itemdesc1.Substring(itemdesc1.IndexOf(")") + 2);
                //    }
                //    string magicdesc0 = magic[item.Action2Value1_Art1].Desc.Split('\n')[0];
                //    string magicdesc1 = magic[item.Action2Value1_Art1].Desc.Split('\n')[1];
                //    if (itemdesc0 != magicdesc0) {
                //        Console.WriteLine();
                //        Console.WriteLine(item.Name);
                //        Console.WriteLine(itemdesc0);
                //        Console.WriteLine(magicdesc0);
                //        Console.WriteLine();
                //    }
                //    if (itemdesc1 != magicdesc1) {
                //        Console.WriteLine();
                //        Console.WriteLine(item.Name);
                //        Console.WriteLine(itemdesc1);
                //        Console.WriteLine(magicdesc1);
                //        Console.WriteLine();
                //    }
                //}
                // clang-format on

                // series consistency: add magic power class to description
                if (isSingleArt) {
                    size_t where = item.Desc.find_first_of(']');
                    if (where != std::string::npos) {
                        const auto it = magic.find(item.Action2Value1_Art1);
                        if (it != magic.end()) {
                            const auto& m = it->second;
                            if (m.Effect1_Type == 0x01 || m.Effect1_Type == 0x02
                                || m.Effect1_Type == 0x70
                                || (m.Effect1_Type >= 0xd9 && m.Effect1_Type <= 0xdd)
                                || m.Effect1_Type == 0xdf) {
                                item.Desc =
                                    Insert(item.Desc, where, GetMagicClass(m.Effect1_Value1));
                                item.Desc = Insert(item.Desc, where, " - Class ");
                            }
                        }
                    }
                }

                // series consistency: quartz that just boost a stat and nothing else should say
                // Stat Boost
                if (IsStatusQuartz(item)) {
                    item.Desc = Insert(item.Desc, 5, "Stat Boost");
                }

                // some quartz have incorrect (R)/(SR) in description
                FixRarenessIdentifierInDescription(item);

                // series consistency: quartz that have passive effects should say Special
                if (IsSpecialQuartz(item)) {
                    item.Desc =
                        Insert(item.Desc,
                               item.Action2_Rarity == 0 ? 5 : (item.Action2_Rarity == 1 ? 8 : 9),
                               "Special");
                }

                entry.Data = item.ToBinary();
            }
        }
    }

    // sync inconsistent descriptions across quartz/magic
    SyncEffect(item_tbl, 609, magic_tbl, 33, false);      // Heat Up
    SyncEffect(item_tbl, 624, magic_tbl, 33, false);      // Heat Up R
    SyncEffect(item_tbl, 685, magic_tbl, 48, true);       // Chrono Drive R
    SyncEffect(item_tbl, 673, magic_tbl, 50, true);       // Chrono Burst
    SyncEffect(item_tbl, 687, magic_tbl, 50, true);       // Chrono Burst R
    SyncEffect(item_tbl, 708, magic_tbl, 53, true);       // Cross Crusade R
    SyncEffect(item_tbl, 709, magic_tbl, 54, true);       // Altair Cannon R
    SyncEffect(item_tbl, 699, magic_tbl, 55, true);       // Fortuna
    SyncEffect(item_tbl, 710, magic_tbl, 55, true);       // Fortuna R
    SyncEffect(item_tbl, 706, magic_tbl, 51, true);       // Golden Sphere R
    SyncEffect(item_tbl, 707, magic_tbl, 52, true);       // Dark Matter R
    SyncEffect(item_tbl, 711, magic_tbl, 56, true);       // Shining R
    SyncEffect(item_tbl, 712, magic_tbl, 57, true);       // Seraphic Ring R
    SyncEffect(item_tbl, 725, magic_tbl, 60, false);      // Phantom Phobia
    SyncEffect(item_tbl, 737, magic_tbl, 60, false);      // Phantom Phobia R
    SyncEffect(item_tbl, 728, magic_tbl, 63, false);      // Saintly Force
    SyncEffect(item_tbl, 740, magic_tbl, 63, false);      // Saintly Force R
    SyncEffect(item_tbl, 729, magic_tbl, 64, true);       // Crescent Mirror
    SyncEffect(item_tbl, 741, magic_tbl, 64, true);       // Crescent Mirror R
    SyncDescription(item_tbl, 543, magic_tbl, 13, false); // Crest
    SyncDescription(item_tbl, 543, magic_tbl, 13, false); // Crest R
    SyncDescription(item_tbl, 557, magic_tbl, 14, false); // La Crest
    SyncDescription(item_tbl, 558, magic_tbl, 14, false); // La Crest R
    SyncDescription(item_tbl, 545, magic_tbl, 15, true);  // Adamantine Shield
    SyncDescription(item_tbl, 559, magic_tbl, 15, true);  // Adamantine Shield R
    SyncDescription(item_tbl, 605, magic_tbl, 29, false); // Volcanic Rain
    SyncDescription(item_tbl, 620, magic_tbl, 29, false); // Volcanic Rain R
    SyncDescription(item_tbl, 607, magic_tbl, 31, true);  // Purgatorial Flame
    SyncDescription(item_tbl, 622, magic_tbl, 31, true);  // Purgatorial Flame R
    SyncDescription(item_tbl, 609, magic_tbl, 33, false); // Heat Up
    SyncDescription(item_tbl, 624, magic_tbl, 33, false); // Heat Up R
    SyncDescription(item_tbl, 610, magic_tbl, 34, true);  // Forte
    SyncDescription(item_tbl, 625, magic_tbl, 34, true);  // Forte R
    SyncDescription(item_tbl, 611, magic_tbl, 35, true);  // La Forte
    SyncDescription(item_tbl, 626, magic_tbl, 35, true);  // La Forte R
    SyncDescription(item_tbl, 639, magic_tbl, 40, true);  // Ragna Vortex
    SyncDescription(item_tbl, 654, magic_tbl, 40, true);  // Ragna Vortex R
    SyncDescription(item_tbl, 667, magic_tbl, 44, true);  // Soul Blur
    SyncDescription(item_tbl, 681, magic_tbl, 44, true);  // Soul Blur R
    SyncDescription(item_tbl, 668, magic_tbl, 45, true);  // Demonic Scythe
    SyncDescription(item_tbl, 682, magic_tbl, 45, true);  // Demonic Scythe R
    SyncDescription(item_tbl, 669, magic_tbl, 46, true);  // Grim Butterfly
    SyncDescription(item_tbl, 683, magic_tbl, 46, true);  // Grim Butterfly R
    SyncDescription(item_tbl, 673, magic_tbl, 50, true);  // Chrono Burst
    SyncDescription(item_tbl, 687, magic_tbl, 50, true);  // Chrono Burst R
    SyncDescription(item_tbl, 697, magic_tbl, 53, true);  // Cross Crusade
    SyncDescription(item_tbl, 708, magic_tbl, 53, true);  // Cross Crusade R
    SyncDescription(item_tbl, 698, magic_tbl, 54, false); // Altair Cannon
    SyncDescription(item_tbl, 709, magic_tbl, 54, false); // Altair Cannon R
    SyncDescription(item_tbl, 699, magic_tbl, 55, true);  // Fortuna
    SyncDescription(item_tbl, 710, magic_tbl, 55, true);  // Fortuna R
    SyncDescription(item_tbl, 726, magic_tbl, 61, true);  // Claiomh Solarion
    SyncDescription(item_tbl, 738, magic_tbl, 61, true);  // Claiomh Solarion R
}

std::string_view GetDescription() {
    return "Series consistency fixes in EN item and magic descriptions.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file_item = getCheckedFile(
            "data/text/dat_us/t_item.tbl",
            136541,
            SenPatcher::SHA1FromHexString("b64ec4d8b62042166e97e60c575552039c49c465"));
        auto file_magic = getCheckedFile(
            "data/text/dat_us/t_magic.tbl",
            24136,
            SenPatcher::SHA1FromHexString("d5f7bf4c4c575efd5699e8bbd4040b81276a7284"));
        if (!file_item || !file_magic) {
            return false;
        }

        auto& bin_item = file_item->Data;
        auto& bin_magic = file_magic->Data;
        Tbl tbl_item(
            bin_item.data(), bin_item.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        Tbl tbl_magic(
            bin_magic.data(), bin_magic.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        PatchItemTbl(tbl_item);
        PatchMagicTbl(tbl_magic);
        SyncItemMagicTbl(tbl_item, tbl_magic);

        {
            std::vector<char> bin2;
            MemoryStream ms(bin2);
            tbl_item.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
            result.emplace_back(
                std::move(bin2), file_item->Filename, SenPatcher::P3ACompressionType::LZ4);
        }
        {
            std::vector<char> bin2;
            MemoryStream ms(bin2);
            tbl_magic.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
            result.emplace_back(
                std::move(bin2), file_magic->Filename, SenPatcher::P3ACompressionType::LZ4);
        }

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::text_dat_us_t_item_tbl_t_magic_tbl
