#include "tbl.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen1 {
Tbl::Tbl(const char* buffer,
         size_t length,
         HyoutaUtils::EndianUtils::Endianness e,
         HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    SenLib::DuplicatableByteArrayStream stream(buffer, length);
    uint16_t entryCount = stream.ReadUInt16(e);

    std::vector<TblEntry> entries;
    entries.reserve(entryCount);
    for (int i = 0; i < entryCount; ++i) {
        auto& d = entries.emplace_back();
        d.Name = stream.ReadNulltermString(encoding);
        uint16_t count = GetLength(d.Name, stream, e, encoding);
        d.Data = stream.ReadVector(count);
    }

    Entries = std::move(entries);
}

void Tbl::WriteToStream(WriteStream& s,
                        HyoutaUtils::EndianUtils::Endianness e,
                        HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    s.WriteUInt16((uint16_t)Entries.size(), e);
    for (TblEntry& entry : Entries) {
        s.WriteNulltermString(entry.Name, encoding);
        s.WriteUInt16((uint16_t)entry.Data.size(), e);
        s.WriteArray(entry.Data);
    }
}

uint16_t Tbl::GetLength(const std::string& name,
                        ReadStream& stream,
                        HyoutaUtils::EndianUtils::Endianness e,
                        HyoutaUtils::TextUtils::GameTextEncoding encoding) const {
    if (name == "item") {
        stream.ReadUInt16(e);
        auto p = stream.GetPosition();
        stream.DiscardBytes(4);
        stream.ReadNulltermString(encoding);
        stream.DiscardBytes(46);
        stream.ReadNulltermString(encoding);
        stream.ReadNulltermString(encoding);
        auto l = stream.GetPosition() - p;
        stream.SetPosition(p);
        return static_cast<uint16_t>(l);
    } else if (name == "magic") {
        stream.ReadUInt16(e);
        auto p = stream.GetPosition();
        stream.DiscardBytes(4);
        stream.ReadNulltermString(encoding);
        stream.DiscardBytes(24);
        stream.ReadNulltermString(encoding);
        stream.ReadNulltermString(encoding);
        stream.ReadNulltermString(encoding);
        auto l = stream.GetPosition() - p;
        stream.SetPosition(p);
        return static_cast<uint16_t>(l);
    }
    return stream.ReadUInt16(e);
}

ItemData::ItemData(const char* data,
                   size_t dataLength,
                   HyoutaUtils::EndianUtils::Endianness endian,
                   HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16(endian);
    Usability = stream.ReadUInt16(endian);
    Flags = stream.ReadNulltermString(encoding);
    ItemType = stream.ReadUInt8();
    Element = stream.ReadUInt8();
    Status_Breaker = stream.ReadUInt8();
    Range = stream.ReadUInt8();
    AttackArea = stream.ReadUInt8();
    Action1_PassiveEffect = stream.ReadUInt8();
    Action1Value1_Art2 = stream.ReadUInt16(endian);
    Action1Value2_Art3 = stream.ReadUInt16(endian);
    Action2_Rarity = stream.ReadUInt8();
    Action2Value1_Art1 = stream.ReadUInt16(endian);
    Action2Value2_ProcChance = stream.ReadUInt16(endian);
    STR = stream.ReadUInt16(endian);
    DEF = stream.ReadUInt16(endian);
    ATS = stream.ReadUInt16(endian);
    ADF = stream.ReadUInt16(endian);
    ACC = stream.ReadUInt16(endian);
    EVA = stream.ReadUInt16(endian);
    SPD = stream.ReadUInt16(endian);
    MOV = stream.ReadUInt16(endian);
    HP = stream.ReadUInt16(endian);
    EP = stream.ReadUInt16(endian);
    Mira = stream.ReadUInt32(endian);
    CarryLimit = stream.ReadUInt8();
    SortOrder = stream.ReadUInt16(endian);
    Unknown1 = stream.ReadUInt16(endian);
    Unknown2 = stream.ReadUInt16(endian);
    Name = stream.ReadNulltermString(encoding);
    Desc = stream.ReadNulltermString(encoding);
}

std::vector<char> ItemData::ToBinary(HyoutaUtils::EndianUtils::Endianness endian,
                                     HyoutaUtils::TextUtils::GameTextEncoding encoding) const {
    std::vector<char> rv;
    {
        MemoryStream ms(rv);
        ms.WriteUInt16(Idx, endian);
        ms.WriteUInt16(Usability, endian);
        ms.WriteNulltermString(Flags, encoding);
        ms.WriteUInt8(ItemType);
        ms.WriteUInt8(Element);
        ms.WriteUInt8(Status_Breaker);
        ms.WriteUInt8(Range);
        ms.WriteUInt8(AttackArea);
        ms.WriteUInt8(Action1_PassiveEffect);
        ms.WriteUInt16(Action1Value1_Art2, endian);
        ms.WriteUInt16(Action1Value2_Art3, endian);
        ms.WriteUInt8(Action2_Rarity);
        ms.WriteUInt16(Action2Value1_Art1, endian);
        ms.WriteUInt16(Action2Value2_ProcChance, endian);
        ms.WriteUInt16(STR, endian);
        ms.WriteUInt16(DEF, endian);
        ms.WriteUInt16(ATS, endian);
        ms.WriteUInt16(ADF, endian);
        ms.WriteUInt16(ACC, endian);
        ms.WriteUInt16(EVA, endian);
        ms.WriteUInt16(SPD, endian);
        ms.WriteUInt16(MOV, endian);
        ms.WriteUInt16(HP, endian);
        ms.WriteUInt16(EP, endian);
        ms.WriteUInt32(Mira, endian);
        ms.WriteUInt8(CarryLimit);
        ms.WriteUInt16(SortOrder, endian);
        ms.WriteUInt16(Unknown1, endian);
        ms.WriteUInt16(Unknown2, endian);
        ms.WriteNulltermString(Name, encoding);
        ms.WriteNulltermString(Desc, encoding);
    }
    return rv;
}

MagicData::MagicData(const char* data,
                     size_t dataLength,
                     HyoutaUtils::EndianUtils::Endianness endian,
                     HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16(endian);
    Unknown0 = stream.ReadUInt16(endian);
    Flags = stream.ReadNulltermString(encoding);
    Unknown1a = stream.ReadUInt8();
    Unknown1b = stream.ReadUInt8();
    Unknown1c = stream.ReadUInt8();
    Unknown1d = stream.ReadUInt8();
    Unknown1e = stream.ReadUInt8();
    Range = stream.ReadUInt8();
    Effect1_Type = stream.ReadUInt8();
    Effect1_Value1 = stream.ReadUInt16(endian);
    Effect1_Value2 = stream.ReadUInt16(endian);
    Effect2_Type = stream.ReadUInt8();
    Effect2_Value1 = stream.ReadUInt16(endian);
    Effect2_Value2 = stream.ReadUInt16(endian);
    Unknown5e = stream.ReadUInt8();
    Unknown6a = stream.ReadUInt8();
    Unknown6b = stream.ReadUInt16(endian);
    Unbalance = stream.ReadUInt8();
    Unknown6d = stream.ReadUInt8();
    Unknown6e = stream.ReadUInt16(endian);
    Anim = stream.ReadNulltermString(encoding);
    Name = stream.ReadNulltermString(encoding);
    Desc = stream.ReadNulltermString(encoding);
}

std::vector<char> MagicData::ToBinary() const {
    std::vector<char> rv;
    {
        MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown0);
        ms.WriteUTF8Nullterm(Flags);
        ms.WriteUInt8(Unknown1a);
        ms.WriteUInt8(Unknown1b);
        ms.WriteUInt8(Unknown1c);
        ms.WriteUInt8(Unknown1d);
        ms.WriteUInt8(Unknown1e);
        ms.WriteUInt8(Range);
        ms.WriteUInt8(Effect1_Type);
        ms.WriteUInt16(Effect1_Value1);
        ms.WriteUInt16(Effect1_Value2);
        ms.WriteUInt8(Effect2_Type);
        ms.WriteUInt16(Effect2_Value1);
        ms.WriteUInt16(Effect2_Value2);
        ms.WriteUInt8(Unknown5e);
        ms.WriteUInt8(Unknown6a);
        ms.WriteUInt16(Unknown6b);
        ms.WriteUInt8(Unbalance);
        ms.WriteUInt8(Unknown6d);
        ms.WriteUInt16(Unknown6e);
        ms.WriteUTF8Nullterm(Anim);
        ms.WriteUTF8Nullterm(Name);
        ms.WriteUTF8Nullterm(Desc);
    }
    return rv;
}

CookData::CookData(const char* data, size_t dataLength) {
    DuplicatableByteArrayStream stream(data, dataLength);
    name = stream.ReadUTF8Nullterm();
    d1 = stream.ReadArray<0x22>();
    item1 = stream.ReadUInt16();
    item1line1 = stream.ReadUTF8Nullterm();
    item1line2 = stream.ReadUTF8Nullterm();
    item2 = stream.ReadUInt16();
    item2line1 = stream.ReadUTF8Nullterm();
    item2line2 = stream.ReadUTF8Nullterm();
    item3 = stream.ReadUInt16();
    item3line1 = stream.ReadUTF8Nullterm();
    item3line2 = stream.ReadUTF8Nullterm();
    item4 = stream.ReadUInt16();
    item4line1 = stream.ReadUTF8Nullterm();
    item4line2 = stream.ReadUTF8Nullterm();
    const size_t dlen = static_cast<size_t>(dataLength - stream.GetPosition());
    d2.resize(dlen);
    stream.Read(d2.data(), dlen);
}

std::vector<char> CookData::ToBinary() const {
    std::vector<char> rv;
    {
        MemoryStream ms(rv);
        ms.WriteUTF8Nullterm(name);
        ms.Write(d1.data(), d1.size());
        ms.WriteUInt16(item1);
        ms.WriteUTF8Nullterm(item1line1);
        ms.WriteUTF8Nullterm(item1line2);
        ms.WriteUInt16(item2);
        ms.WriteUTF8Nullterm(item2line1);
        ms.WriteUTF8Nullterm(item2line2);
        ms.WriteUInt16(item3);
        ms.WriteUTF8Nullterm(item3line1);
        ms.WriteUTF8Nullterm(item3line2);
        ms.WriteUInt16(item4);
        ms.WriteUTF8Nullterm(item4line1);
        ms.WriteUTF8Nullterm(item4line2);
        ms.Write(d2.data(), d2.size());
    }
    return rv;
}

VoiceData::VoiceData(const char* data,
                     size_t dataLength,
                     HyoutaUtils::EndianUtils::Endianness e,
                     HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    DuplicatableByteArrayStream stream(data, dataLength);
    Index = stream.ReadUInt16(e);
    Name = stream.ReadNulltermString(encoding);
    Unknown1 = stream.ReadUInt64(e);
    Unknown2 = stream.ReadUInt16(e);
    Unknown3 = stream.ReadUInt32(e);
}

std::vector<char> VoiceData::ToBinary(HyoutaUtils::EndianUtils::Endianness e,
                                      HyoutaUtils::TextUtils::GameTextEncoding encoding) const {
    std::vector<char> rv;
    {
        MemoryStream ms(rv);
        ms.WriteUInt16(Index, e);
        ms.WriteNulltermString(Name, encoding);
        ms.WriteUInt64(Unknown1, e);
        ms.WriteUInt16(Unknown2, e);
        ms.WriteUInt32(Unknown3, e);
    }
    return rv;
}
} // namespace SenLib::Sen1
