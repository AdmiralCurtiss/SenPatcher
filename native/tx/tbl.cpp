#include "tbl.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::TX {
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

TextTableData::TextTableData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Str = stream.ReadUTF8Nullterm();
    assert(stream.GetPosition() == dataLength);
}

std::vector<char> TextTableData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUTF8Nullterm(Str);
    }
    return rv;
}

ItemHelpData::ItemHelpData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Str = stream.ReadUTF8Nullterm();
    D = stream.ReadArray<9>();
    assert(stream.GetPosition() == dataLength);
}

std::vector<char> ItemHelpData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUTF8Nullterm(Str);
        ms.Write(D.data(), D.size());
    }
    return rv;
}

hkitugi_lst::hkitugi_lst(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown1 = stream.ReadUInt16();
    Unknown2 = stream.ReadUInt16();
    Name = stream.ReadUTF8Nullterm();
    Description = stream.ReadUTF8Nullterm();
    assert(stream.GetPosition() == dataLength);
}

std::vector<char> hkitugi_lst::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown1);
        ms.WriteUInt16(Unknown2);
        ms.WriteUTF8Nullterm(Name);
        ms.WriteUTF8Nullterm(Description);
    }
    return rv;
}

ActiveVoiceTableData::ActiveVoiceTableData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown1 = stream.ReadUInt16();
    Unknown2 = stream.ReadUInt16();
    Image = stream.ReadUTF8Nullterm();
    Unknown3 = stream.ReadUInt16();
    Unknown4 = stream.ReadUInt16();
    Text = stream.ReadUTF8Nullterm();
    Unknown5 = stream.ReadUInt32();
    Unknown6 = stream.ReadUInt32();
    Unknown7 = stream.ReadUInt16();
    Unknown8 = stream.ReadUInt16();
    Unknown9 = stream.ReadUInt16();
    Unknown10 = stream.ReadUInt16();
    Unknown11 = stream.ReadUInt16();
    assert(stream.GetPosition() == dataLength);
}

std::vector<char> ActiveVoiceTableData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown1);
        ms.WriteUInt16(Unknown2);
        ms.WriteUTF8Nullterm(Image);
        ms.WriteUInt16(Unknown3);
        ms.WriteUInt16(Unknown4);
        ms.WriteUTF8Nullterm(Text);
        ms.WriteUInt32(Unknown5);
        ms.WriteUInt32(Unknown6);
        ms.WriteUInt16(Unknown7);
        ms.WriteUInt16(Unknown8);
        ms.WriteUInt16(Unknown9);
        ms.WriteUInt16(Unknown10);
        ms.WriteUInt16(Unknown11);
    }
    return rv;
}

DungeonData::DungeonData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown1 = stream.ReadUInt16();
    Unknown2 = stream.ReadUInt8();
    Name = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    D.resize(dlen);
    stream.Read(D.data(), dlen);
}

std::vector<char> DungeonData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown1);
        ms.WriteUInt8(Unknown2);
        ms.WriteUTF8Nullterm(Name);
        ms.Write(D.data(), D.size());
    }
    return rv;
}

ItemData::ItemData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown1 = stream.ReadUInt16();
    Flags = stream.ReadUTF8Nullterm();
    Unknown2 = stream.ReadArray<0x3e>();
    Name = stream.ReadUTF8Nullterm();
    Description = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    D.resize(dlen);
    stream.Read(D.data(), dlen);
}

std::vector<char> ItemData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown1);
        ms.WriteUTF8Nullterm(Flags);
        ms.WriteArray(Unknown2);
        ms.WriteUTF8Nullterm(Name);
        ms.WriteUTF8Nullterm(Description);
        ms.Write(D.data(), D.size());
    }
    return rv;
}

MapJumpData::MapJumpData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown1 = stream.ReadUInt16();
    Name = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    D.resize(dlen);
    stream.Read(D.data(), dlen);
}

std::vector<char> MapJumpData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown1);
        ms.WriteUTF8Nullterm(Name);
        ms.Write(D.data(), D.size());
    }
    return rv;
}

MagicData::MagicData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown1 = stream.ReadUInt16();
    Flags = stream.ReadUTF8Nullterm();
    Unknown2 = stream.ReadArray<0x33>();
    Name = stream.ReadUTF8Nullterm();
    Description = stream.ReadUTF8Nullterm();
    assert(stream.GetPosition() == dataLength);
}

std::vector<char> MagicData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown1);
        ms.WriteUTF8Nullterm(Flags);
        ms.WriteArray(Unknown2);
        ms.WriteUTF8Nullterm(Name);
        ms.WriteUTF8Nullterm(Description);
    }
    return rv;
}

QSText::QSText(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown1 = stream.ReadUInt8();
    Text = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    D.resize(dlen);
    stream.Read(D.data(), dlen);
}

std::vector<char> QSText::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt8(Unknown1);
        ms.WriteUTF8Nullterm(Text);
        ms.Write(D.data(), D.size());
    }
    return rv;
}

QSChar::QSChar(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown1 = stream.ReadUInt16();
    Unknown2 = stream.ReadUInt16();
    Unknown3 = stream.ReadUInt8();
    for (size_t i = 0; i < Strings.size(); ++i) {
        Strings[i] = stream.ReadUTF8Nullterm();
    }
    assert(stream.GetPosition() == dataLength);
}

std::vector<char> QSChar::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown1);
        ms.WriteUInt16(Unknown2);
        ms.WriteUInt8(Unknown3);
        for (size_t i = 0; i < Strings.size(); ++i) {
            ms.WriteUTF8Nullterm(Strings[i]);
        }
    }
    return rv;
}

QSStage::QSStage(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown1 = stream.ReadUInt16();
    Name = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    D.resize(dlen);
    stream.Read(D.data(), dlen);
}

std::vector<char> QSStage::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown1);
        ms.WriteUTF8Nullterm(Name);
        ms.Write(D.data(), D.size());
    }
    return rv;
}

PlaceTableData::PlaceTableData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    Idx = stream.ReadUInt16();
    Unknown1 = stream.ReadUInt16();
    Map = stream.ReadUTF8Nullterm();
    Name = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    D.resize(dlen);
    stream.Read(D.data(), dlen);
}

std::vector<char> PlaceTableData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(Idx);
        ms.WriteUInt16(Unknown1);
        ms.WriteUTF8Nullterm(Map);
        ms.WriteUTF8Nullterm(Name);
        ms.Write(D.data(), D.size());
    }
    return rv;
}

} // namespace SenLib::TX
