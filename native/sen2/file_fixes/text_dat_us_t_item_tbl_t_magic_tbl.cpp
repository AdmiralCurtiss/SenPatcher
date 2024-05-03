#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen2/tbl.h"
#include "util/hash/sha1.h"
#include "util/text.h"

#include "text_dat_us_t_item_tbl_t_magic_tbl.h"

namespace SenLib::Sen2::FileFixes::text_dat_us_t_item_tbl_t_magic_tbl {
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

static void PatchItemTbl(Tbl& tbl) {
    using namespace HyoutaUtils::TextUtils;

    // List<ItemData> items = new List<ItemData>();
    // foreach (TblEntry entry in tbl.Entries) {
    //     if (entry.Name == "item" || entry.Name == "item_q") {
    //         items.Add(new ItemData(entry.Data, entry.Name == "item_q"));
    //     }
    // }
    // foreach (TblEntry entry in tbl.Entries) {
    //     if (entry.Name == "item" || entry.Name == "item_q") {
    //         ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
    //         item.Flags = item.Flags.Replace("Z", "");
    //         entry.Data = item.ToBinary();
    //     }
    // }

    for (TblEntry& entry : tbl.Entries) {
        if (entry.Name == "item" || entry.Name == "item_q") {
            ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
            item.Desc = FixHpEpCpSpacing(item.Desc);
            entry.Data = item.ToBinary();
        }
    }

    {
        // clarify Cheese Curry Noodles
        auto& entry = tbl.Entries[999];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        item.Desc = InsertSubstring(item.Desc, 22, item.Desc, 31, 10);
        entry.Data = item.ToBinary();
    }

    {
        // missing gender lock on some staves
        auto m = ItemData(tbl.Entries[327].Data.data(), tbl.Entries[327].Data.size(), false)
                     .Desc.substr(14, 9);
        auto f = ItemData(tbl.Entries[328].Data.data(), tbl.Entries[328].Data.size(), false)
                     .Desc.substr(14, 11);
        {
            auto& entry = tbl.Entries[116];
            ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
            item.Desc = Insert(item.Desc, 21, m);
            entry.Data = item.ToBinary();
        }
        {
            auto& entry = tbl.Entries[117];
            ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
            item.Desc = Insert(item.Desc, 29, m);
            entry.Data = item.ToBinary();
        }
        {
            auto& entry = tbl.Entries[119];
            ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
            item.Desc = Insert(item.Desc, 31, m);
            entry.Data = item.ToBinary();
        }
        {
            auto& entry = tbl.Entries[121];
            ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
            item.Desc = Insert(item.Desc, 21, f);
            entry.Data = item.ToBinary();
        }
        {
            auto& entry = tbl.Entries[122];
            ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
            item.Desc = Insert(item.Desc, 29, f);
            entry.Data = item.ToBinary();
        }
        {
            auto& entry = tbl.Entries[124];
            ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
            item.Desc = Insert(item.Desc, 31, f);
            entry.Data = item.ToBinary();
        }
    }

    {
        // wrong buff order in Crystal Dress
        auto& entry = tbl.Entries[294];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        item.Desc = InsertSubstring(item.Desc, 22, item.Desc, 7, 7);
        item.Desc = Remove(item.Desc, 7, 7);
        entry.Data = item.ToBinary();
    }

    {
        // extra space in Yaksha
        auto& entry = tbl.Entries[587];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        item.Desc = Remove(item.Desc, 9, 1);
        entry.Data = item.ToBinary();
    }

    {
        // capitalization inconsistency in Heat Up
        auto& entry = tbl.Entries[597];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        item.Desc = ReplaceSubstring(item.Desc, 32, 9, item.Desc, 57, 9);
        entry.Data = item.ToBinary();
    }

    {
        // capitalization inconsistency in Heat Up R
        auto& entry = tbl.Entries[613];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        item.Desc = ReplaceSubstring(item.Desc, 35, 9, item.Desc, 69, 9);
        entry.Data = item.ToBinary();
    }

    {
        // missing turn count for buff in Dark Matter/R
        auto other = ItemData(tbl.Entries[698].Data.data(),
                              tbl.Entries[698].Data.size(),
                              tbl.Entries[698].Name == "item_q")
                         .Desc.substr(28, 10);
        auto& entry = tbl.Entries[694];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        item.Desc = InsertSubstring(item.Desc, 48, other, 0, other.size());
        entry.Data = item.ToBinary();
        auto& entry2 = tbl.Entries[707];
        ItemData item2(entry2.Data.data(), entry2.Data.size(), entry2.Name == "item_q");
        item2.Desc = InsertSubstring(item2.Desc, 51, other, 0, other.size());
        entry2.Data = item2.ToBinary();
    }

    {
        // missing turn count for buff in Seraphic Ring/R
        auto other = ItemData(tbl.Entries[955].Data.data(),
                              tbl.Entries[955].Data.size(),
                              tbl.Entries[955].Name == "item_q")
                         .Desc.substr(22, 10);
        auto& entry = tbl.Entries[699];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        item.Desc = InsertSubstring(item.Desc, 60, other, 0, other.size());
        entry.Data = item.ToBinary();
        auto& entry2 = tbl.Entries[712];
        ItemData item2(entry2.Data.data(), entry2.Data.size(), entry2.Name == "item_q");
        item2.Desc = InsertSubstring(item2.Desc, 63, other, 0, other.size());
        entry2.Data = item2.ToBinary();
    }

    {
        // awkward phrasing on Revolution
        auto& entry = tbl.Entries[545];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        std::string str = item.Desc.substr(45, 7);
        item.Desc = InsertSubstring(Remove(item.Desc, 45, 7), 52, str, 0, str.size());
        entry.Data = item.ToBinary();
    }

    {
        // awkward phrasing on Waterfall
        auto& entry = tbl.Entries[586];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        std::string str = item.Desc.substr(45, 6);
        item.Desc = InsertSubstring(Remove(item.Desc, 45, 6), 52, str, 0, str.size());
        entry.Data = item.ToBinary();
    }

    {
        // missing space on Acala Gem
        auto& entry = tbl.Entries[548];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        item.Desc = Insert(item.Desc, 9, " ");
        entry.Data = item.ToBinary();
    }

    {
        // Chrono Burst 'Attack twice' -> 'Act twice'
        auto& entry = tbl.Entries[669];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        item.Desc = ReplaceSubstring(item.Desc, 23, 5, item.Desc, 88, 2);
        entry.Data = item.ToBinary();
    }
    {
        // Chrono Burst R 'Attack twice' -> 'Act twice'
        auto& entry = tbl.Entries[685];
        ItemData item(entry.Data.data(), entry.Data.size(), entry.Name == "item_q");
        item.Desc = ReplaceSubstring(item.Desc, 26, 5, item.Desc, 99, 2);
        entry.Data = item.ToBinary();
    }
}

static const char* GetPhysicalClass(uint32_t value) {
    const char* c = "D";
    if (value >= 110)
        c = "C";
    if (value >= 115)
        c = "C+";
    if (value >= 120)
        c = "B";
    if (value >= 125)
        c = "B+";
    if (value >= 130)
        c = "A";
    if (value >= 135)
        c = "A+";
    if (value >= 140)
        c = "S";
    if (value >= 150)
        c = "S+";
    if (value >= 250)
        c = "SS";
    if (value >= 300)
        c = "SS+";
    if (value >= 350)
        c = "SSS";
    if (value >= 400)
        c = "SSS+";
    if (value >= 450)
        c = "4S";
    if (value >= 500)
        c = "4S+";
    return c;
}
static const char* GetMagicClass(uint32_t value) {
    const char* c = "D";
    if (value >= 120)
        c = "C";
    if (value >= 135)
        c = "C+";
    if (value >= 150)
        c = "B";
    if (value >= 165)
        c = "B+";
    if (value >= 180)
        c = "A";
    if (value >= 195)
        c = "A+";
    if (value >= 210)
        c = "S";
    if (value >= 225)
        c = "S+";
    if (value >= 250)
        c = "SS";
    if (value >= 300)
        c = "SS+";
    if (value >= 350)
        c = "SSS";
    if (value >= 400)
        c = "SSS+";
    if (value >= 450)
        c = "4S";
    if (value >= 600)
        c = "5S";
    return c;
}

static void PatchMagicTbl(Tbl& tbl) {
    using namespace HyoutaUtils::TextUtils;

    // fix typo in Emma's S-Craft
    {
        auto& entry = tbl.Entries[144];
        std::swap(entry.Data[0x48], entry.Data[0x4a]);
    }

    // remove misplaced (R) from Curia
    {
        auto& entry = tbl.Entries[29];
        MagicData item(entry.Data.data(), entry.Data.size());
        item.Desc = Remove(item.Desc, 5, 3);
        entry.Data = item.ToBinary();
    }

    // List<MagicData> items = new List<MagicData>();
    // foreach (TblEntry entry in tbl.Entries) {
    //     if (entry.Name == "magic") {
    //         items.Add(new MagicData(entry.Data));
    //     }
    // }
    // uint attackPowerCounter = 100;
    // foreach (TblEntry entry in tbl.Entries) {
    //     if (entry.Name == "magic") {
    //         MagicData item(entry.Data.data(), entry.Data.size());
    //         item.Flags = item.Flags.Replace("Z", "");
    //         if (item.Effect1_Type == 0x01) {
    //             item.Effect1_Value1 = attackPowerCounter;
    //             item.Desc = "Power == " + attackPowerCounter.ToString();
    //             attackPowerCounter += 10;
    //             item.Range = 6;
    //         }
    //         entry.Data = item.ToBinary();
    //     }
    // }

    // clang-format off
    //var sb = new System.Text.StringBuilder();
    //foreach (TblEntry entry in tbl.Entries) {
    //    if (entry.Name == "magic") {
    //        MagicData item(entry.Data.data(), entry.Data.size());
    //        if (item.Effect1_Type == 0x01 || item.Effect1_Type == 0x02 || item.Effect1_Type == 0x70) {
    //            bool isMagic = item.Effect1_Type == 0x02 || item.Effect1_Type == 0x70;
    //            sb.AppendFormat("{0} / Power {1} / {2} / Craft Class {3} / Art Class {4} / In-Game Description: {5}\n",
    //                item.Name, item.Effect1_Value1, isMagic ? "Magic" : "Physical", GetPhysicalClass(item.Effect1_Value1),
    //                GetMagicClass(item.Effect1_Value1), item.Desc.Replace("\n", "{n}")
    //            );
    //        } else {
    //            //sb.AppendFormat("{0} / In-Game Description: {1}\n", item.Name, item.Desc.Replace("\n", "{n}"));
    //        }
    //    }
    //}
    //File.WriteAllText(@"c:\__ed8\__script-compare_cs2\magic-classes.txt", sb.ToString(), System.Text.Encoding.UTF8);
    // clang-format on

    for (TblEntry& entry : tbl.Entries) {
        if (entry.Name == "magic") {
            MagicData item(entry.Data.data(), entry.Data.size());
            item.Desc = FixHpEpCpSpacing(item.Desc);
            entry.Data = item.ToBinary();
        }
    }

    {
        // missing turn count for buff in Dark Matter
        auto other =
            MagicData(tbl.Entries[60].Data.data(), tbl.Entries[60].Data.size()).Desc.substr(28, 10);
        auto& entry = tbl.Entries[56];
        MagicData item(entry.Data.data(), entry.Data.size());
        item.Desc = InsertSubstring(item.Desc, 48, other, 0, other.size());
        entry.Data = item.ToBinary();
    }

    {
        // missing turn count for buff in Seraphic Ring
        auto other =
            MagicData(tbl.Entries[72].Data.data(), tbl.Entries[72].Data.size()).Desc.substr(42, 10);
        auto& entry = tbl.Entries[61];
        MagicData item(entry.Data.data(), entry.Data.size());
        item.Desc = InsertSubstring(item.Desc, 60, other, 0, other.size());
        entry.Data = item.ToBinary();
    }

    {
        // Grail Burst 'Attack twice' -> 'Act twice'
        auto& entry = tbl.Entries[71];
        auto desc = MagicData(tbl.Entries[54].Data.data(), tbl.Entries[54].Data.size()).Desc;
        MagicData item(entry.Data.data(), entry.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 36, 5, desc, 97, 2);
        entry.Data = item.ToBinary();
    }
    {
        // Chrono Burst 'Attack twice' -> 'Act twice'
        auto& entry = tbl.Entries[54];
        MagicData item(entry.Data.data(), entry.Data.size());
        item.Desc = ReplaceSubstring(item.Desc, 23, 5, item.Desc, 97, 2);
        entry.Data = item.ToBinary();
    }

    {
        // Add 'sacred' into Holy Breath's description like CS1
        auto& entry = tbl.Entries[46];
        auto name = MagicData(tbl.Entries[306].Data.data(), tbl.Entries[306].Data.size()).Name;
        MagicData item(entry.Data.data(), entry.Data.size());
        item.Desc = InsertSubstring(item.Desc, 79, name, 1, 6);
        item.Desc = InsertSubstring(item.Desc, 79, item.Desc, 44, 1);
        entry.Data = item.ToBinary();
    }
}

static void SyncDescription(Tbl& itemTbl, int itemId, Tbl& magicTbl, int magicId, bool useMagic) {
    using namespace HyoutaUtils::TextUtils;

    auto& itemEntry = itemTbl.Entries[itemId];
    ItemData item(itemEntry.Data.data(), itemEntry.Data.size(), itemEntry.Name == "item_q");
    auto& magicEntry = magicTbl.Entries[magicId];
    MagicData magic(magicEntry.Data.data(), magicEntry.Data.size());
    size_t itemDescStart = item.Desc.find_last_of('\n') + 1;
    size_t magicDescStart = magic.Desc.find_last_of('\n') + 1;
    if (item.Desc[itemDescStart] == '(') {
        while (item.Desc[itemDescStart] != ')') {
            ++itemDescStart;
        }
        itemDescStart += 2;
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

static void SyncMagicDescriptions(Tbl& itemTbl, Tbl& magicTbl) {
    SyncDescription(itemTbl, 522, magicTbl, 18, true);  // Adamantine Shield
    SyncDescription(itemTbl, 537, magicTbl, 18, true);  // Adamantine Shield R
    SyncDescription(itemTbl, 550, magicTbl, 20, true);  // Frost Edge
    SyncDescription(itemTbl, 569, magicTbl, 20, true);  // Frost Edge R
    SyncDescription(itemTbl, 593, magicTbl, 33, false); // Volcanic Rain
    SyncDescription(itemTbl, 595, magicTbl, 35, true);  // Purgatorial Flame
    SyncDescription(itemTbl, 611, magicTbl, 35, true);  // Purgatorial Flame R
    SyncDescription(itemTbl, 598, magicTbl, 38, true);  // Forte
    SyncDescription(itemTbl, 614, magicTbl, 38, true);  // Forte R
    SyncDescription(itemTbl, 599, magicTbl, 39, true);  // La Forte
    SyncDescription(itemTbl, 615, magicTbl, 39, true);  // La Forte R
    SyncDescription(itemTbl, 632, magicTbl, 44, true);  // Ragna Vortex
    SyncDescription(itemTbl, 648, magicTbl, 44, true);  // Ragna Vortex R
    SyncDescription(itemTbl, 634, magicTbl, 46, true);  // Holy Breath
    SyncDescription(itemTbl, 650, magicTbl, 46, true);  // Holy Breath R
    SyncDescription(itemTbl, 663, magicTbl, 48, true);  // Soul Blur
    SyncDescription(itemTbl, 679, magicTbl, 48, true);  // Soul Blur R
    SyncDescription(itemTbl, 664, magicTbl, 49, true);  // Demonic Scythe
    SyncDescription(itemTbl, 680, magicTbl, 49, true);  // Demonic Scythe R
    SyncDescription(itemTbl, 665, magicTbl, 50, true);  // Grim Butterfly
    SyncDescription(itemTbl, 681, magicTbl, 50, true);  // Grim Butterfly R
    SyncDescription(itemTbl, 669, magicTbl, 54, true);  // Chrono Burst
    SyncDescription(itemTbl, 685, magicTbl, 54, true);  // Chrono Burst R
    SyncDescription(itemTbl, 696, magicTbl, 58, false); // Altair Channon
    SyncDescription(itemTbl, 697, magicTbl, 59, true);  // Fortuna
    SyncDescription(itemTbl, 710, magicTbl, 59, true);  // Fortuna R
    SyncDescription(itemTbl, 726, magicTbl, 65, true);  // Claiomh Solarion
    SyncDescription(itemTbl, 739, magicTbl, 65, true);  // Claiomh Solarion R
}

std::string_view GetDescription() {
    return "Fix minor formatting errors in item descriptions, fix typo in Emma's S-Craft, and sync "
           "art and quartz descriptions.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file_item = getCheckedFile(
            "data/text/dat_us/t_item.tbl",
            191275,
            HyoutaUtils::Hash::SHA1FromHexString("0ab9f575af6113694b18c0128cf1343ac6b48300"));
        auto file_magic = getCheckedFile(
            "data/text/dat_us/t_magic.tbl",
            61489,
            HyoutaUtils::Hash::SHA1FromHexString("92de0d29c0ad4a9ea935870674976924d5df756d"));
        if (!file_item) {
            return false;
        }
        if (!file_magic) {
            return false;
        }

        auto& bin_item = file_item->Data;
        Tbl tbl_item(
            bin_item.data(), bin_item.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        auto& bin_magic = file_magic->Data;
        Tbl tbl_magic(
            bin_magic.data(), bin_magic.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        PatchItemTbl(tbl_item);
        PatchMagicTbl(tbl_magic);
        SyncMagicDescriptions(tbl_item, tbl_magic);

        // clang-format off
        //var magic = new Dictionary<ushort, MagicData>();
        //foreach (var e in magicTbl.Entries) {
        //    if (e.Name == "magic") {
        //        var m = new MagicData(e.Data);
        //        magic.Add(m.Idx, m);
        //    }
        //}
        //foreach (TblEntry entry in tbl.Entries) {
        //    if (entry.Name == "item_q") {
        //        ItemData item(entry.Data.data(), entry.Data.size(), true);
        //        var quartzdata = new DuplicatableByteArrayStream(item.Unknown2);
        //        quartzdata.DiscardBytes(8);
        //        ushort magic0 = quartzdata.ReadUInt16();
        //        ushort magic1 = quartzdata.ReadUInt16();
        //        ushort magic2 = quartzdata.ReadUInt16();
        //        ushort magic3 = quartzdata.ReadUInt16();
        //        ushort magic4 = quartzdata.ReadUInt16();
        //        ushort magic5 = quartzdata.ReadUInt16();
        //        if (magic0 != 0xffff && magic1 == 0xffff && magic2 == 0xffff && magic3 == 0xffff && magic4 == 0xffff && magic5 == 0xffff) {
        //            string itemdesc0 = item.Desc.Split('\n')[0].Replace("(R)", "").Replace("(SR)", "");
        //            string itemdesc1 = item.Desc.Split('\n')[1];
        //            if (itemdesc1.StartsWith("(")) {
        //                itemdesc1 = itemdesc1.Substring(itemdesc1.IndexOf(")") + 2);
        //            }
        //            string magicdesc0 = magic[magic0].Desc.Split('\n')[0];
        //            string magicdesc1 = magic[magic0].Desc.Split('\n')[1];
        //            if (itemdesc0 != magicdesc0) {
        //                Console.WriteLine();
        //                Console.WriteLine(item.Name);
        //                Console.WriteLine(itemdesc0);
        //                Console.WriteLine(magicdesc0);
        //                Console.WriteLine();
        //            }
        //            if (itemdesc1 != magicdesc1) {
        //                Console.WriteLine();
        //                Console.WriteLine(item.Name);
        //                Console.WriteLine(itemdesc1);
        //                Console.WriteLine(magicdesc1);
        //                Console.WriteLine();
        //            }
        //        }
        //    }
        //}
        // clang-format on


        {
            std::vector<char> bin2;
            HyoutaUtils::Stream::MemoryStream ms(bin2);
            tbl_item.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
            result.emplace_back(
                std::move(bin2), file_item->Filename, SenPatcher::P3ACompressionType::LZ4);
        }
        {
            std::vector<char> bin2;
            HyoutaUtils::Stream::MemoryStream ms(bin2);
            tbl_magic.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
            result.emplace_back(
                std::move(bin2), file_magic->Filename, SenPatcher::P3ACompressionType::LZ4);
        }

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::text_dat_us_t_item_tbl_t_magic_tbl
