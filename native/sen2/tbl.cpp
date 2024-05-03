#include "tbl.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen2 {
Tbl::Tbl(const char* buffer,
         size_t length,
         HyoutaUtils::EndianUtils::Endianness e,
         HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(buffer, length);
    uint16_t entryCount = stream.ReadUInt16(e);
    uint32_t definitionCount = stream.ReadUInt32(e);
    std::vector<TblDefinition> definitions;
    definitions.reserve(definitionCount);
    for (uint32_t i = 0; i < definitionCount; ++i) {
        auto& d = definitions.emplace_back();
        d.Name = stream.ReadNulltermString(encoding);
        d.NumberOfEntries = stream.ReadUInt32(e);
    }

    std::vector<TblEntry> entries;
    entries.reserve(entryCount);
    for (int i = 0; i < entryCount; ++i) {
        auto& d = entries.emplace_back();
        d.Name = stream.ReadNulltermString(encoding);
        uint16_t count = GetLength(d.Name, stream, e, encoding);
        d.Data = stream.ReadVector(count);
    }

    Definitions = definitions;
    Entries = entries;
}

void Tbl::RecalcNumberOfEntries() {
    std::unordered_map<std::string_view, TblDefinition*> lookup;
    for (TblDefinition& def : Definitions) {
        lookup.try_emplace(def.Name, &def);
        def.NumberOfEntries = 0;
    }
    for (TblEntry& entry : Entries) {
        auto it = lookup.find(entry.Name);
        if (it != lookup.end()) {
            it->second->NumberOfEntries += 1;
        }
    }
}

void Tbl::WriteToStream(HyoutaUtils::Stream::WriteStream& s,
                        HyoutaUtils::EndianUtils::Endianness e,
                        HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    s.WriteUInt16((uint16_t)Entries.size(), e);
    s.WriteUInt32((uint32_t)Definitions.size(), e);
    for (TblDefinition& def : Definitions) {
        s.WriteNulltermString(def.Name, encoding);
        s.WriteUInt32(def.NumberOfEntries, e);
    }
    for (TblEntry& entry : Entries) {
        s.WriteNulltermString(entry.Name, encoding);
        s.WriteUInt16((uint16_t)entry.Data.size(), e);
        s.WriteArray(entry.Data);
    }
}

uint16_t Tbl::GetLength(const std::string& name,
                        HyoutaUtils::Stream::ReadStream& stream,
                        HyoutaUtils::EndianUtils::Endianness e,
                        HyoutaUtils::TextUtils::GameTextEncoding encoding) const {
    return stream.ReadUInt16(e);
}

ItemData::ItemData(const char* data, size_t dataLength, bool isQuartz) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown0 = stream.ReadUInt16();
    Flags = stream.ReadUTF8Nullterm();
    Unknown1 = stream.ReadArray<0x3c>();
    Name = stream.ReadUTF8Nullterm();
    Desc = stream.ReadUTF8Nullterm();
    Unknown2 = stream.ReadArray<0x8>();
    if (isQuartz) {
        Unknown3_QuartzOnly = stream.ReadArray<0xc>();
    }
    IsQuartz = isQuartz;
}

std::vector<char> ItemData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown0);
        ms.WriteUTF8Nullterm(Flags);
        ms.Write(Unknown1.data(), Unknown1.size());
        ms.WriteUTF8Nullterm(Name);
        ms.WriteUTF8Nullterm(Desc);
        ms.Write(Unknown2.data(), Unknown2.size());
        if (IsQuartz) {
            ms.Write(Unknown3_QuartzOnly.data(), Unknown3_QuartzOnly.size());
        }
    }
    return rv;
}

MagicData::MagicData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown0 = stream.ReadUInt16();
    Flags = stream.ReadUTF8Nullterm();
    Unknown1a = stream.ReadUInt8();
    Unknown1b = stream.ReadUInt8();
    Unknown1c = stream.ReadUInt8();
    Unknown1d = stream.ReadUInt8();
    Unknown1e = stream.ReadUInt8();
    Range = stream.ReadUInt8();
    Effect1_Type = stream.ReadUInt8();
    Effect1_Value1 = stream.ReadUInt32();
    Effect1_Value2 = stream.ReadUInt32();
    Unknown3a = stream.ReadUInt8();
    Unknown3b = stream.ReadUInt8();
    Unknown3c = stream.ReadUInt8();
    Unknown3d = stream.ReadUInt8();
    Effect2_Type = stream.ReadUInt8();
    Effect2_Value1 = stream.ReadUInt32();
    Effect2_Value2 = stream.ReadUInt32();
    Unknown5a = stream.ReadUInt8();
    Unknown5b = stream.ReadUInt8();
    Unknown5c = stream.ReadUInt8();
    Unknown5d = stream.ReadUInt8();
    Unknown5e = stream.ReadUInt8();
    Unknown6a = stream.ReadUInt8();
    Unknown6b = stream.ReadUInt16();
    Unbalance = stream.ReadUInt8();
    Unknown6d = stream.ReadUInt8();
    Unknown6e = stream.ReadUInt16();
    Anim = stream.ReadUTF8Nullterm();
    Name = stream.ReadUTF8Nullterm();
    Desc = stream.ReadUTF8Nullterm();
}

std::vector<char> MagicData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
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
        ms.WriteUInt32(Effect1_Value1);
        ms.WriteUInt32(Effect1_Value2);
        ms.WriteUInt8(Unknown3a);
        ms.WriteUInt8(Unknown3b);
        ms.WriteUInt8(Unknown3c);
        ms.WriteUInt8(Unknown3d);
        ms.WriteUInt8(Effect2_Type);
        ms.WriteUInt32(Effect2_Value1);
        ms.WriteUInt32(Effect2_Value2);
        ms.WriteUInt8(Unknown5a);
        ms.WriteUInt8(Unknown5b);
        ms.WriteUInt8(Unknown5c);
        ms.WriteUInt8(Unknown5d);
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
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
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
        HyoutaUtils::Stream::MemoryStream ms(rv);
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

VoiceData::VoiceData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    idx = stream.ReadUInt16();
    name = stream.ReadUTF8Nullterm();
    unknown1 = stream.ReadUInt64();
    unknown2 = stream.ReadUInt16();
    unknown3 = stream.ReadUInt32();
}

std::vector<char> VoiceData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(idx);
        ms.WriteUTF8Nullterm(name);
        ms.WriteUInt64(unknown1);
        ms.WriteUInt16(unknown2);
        ms.WriteUInt32(unknown3);
    }
    return rv;
}
} // namespace SenLib::Sen2
