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

namespace SenLib::Sen3 {
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
    if (name == "QSTitle") {
        // some of these have incorrect length fields in the official files, manually determine
        // length here...
        stream.DiscardBytes(2);
        uint64_t p = stream.GetPosition();
        stream.DiscardBytes(3);
        stream.ReadNulltermString(encoding);
        stream.ReadNulltermString(encoding);
        stream.DiscardBytes(13);
        uint16_t length = (uint16_t)(stream.GetPosition() - p);
        stream.SetPosition(p);
        return length;
    } else if (name == "NameTableData") {
        // like above, some entries have wrong length
        stream.DiscardBytes(2);
        uint64_t p = stream.GetPosition();
        stream.DiscardBytes(2);
        stream.ReadNulltermString(encoding);
        stream.ReadNulltermString(encoding);
        stream.ReadNulltermString(encoding);
        stream.ReadNulltermString(encoding);
        stream.ReadNulltermString(encoding);
        stream.ReadNulltermString(encoding);
        stream.DiscardBytes(19);
        uint16_t length = (uint16_t)(stream.GetPosition() - p);
        stream.SetPosition(p);
        return length;
    }
    return stream.ReadUInt16(e);
}

TextTableData::TextTableData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    idx = stream.ReadUInt16();
    str = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    d.resize(dlen);
    stream.Read(d.data(), dlen);
}

std::vector<char> TextTableData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(idx);
        ms.WriteUTF8Nullterm(str);
        ms.Write(d.data(), d.size());
    }
    return rv;
}

template<size_t length>
static std::array<uint16_t, length>
    ReadUInt16Array(HyoutaUtils::Stream::DuplicatableByteArrayStream& s,
                    HyoutaUtils::EndianUtils::Endianness endianness =
                        HyoutaUtils::EndianUtils::Endianness::LittleEndian) {
    std::array<uint16_t, length> data;
    for (size_t i = 0; i < length; ++i) {
        data[i] = s.ReadUInt16(endianness);
    }
    return data;
}

static void WriteUInt16Array(HyoutaUtils::Stream::MemoryStream& s,
                             const uint16_t* data,
                             size_t dataLength,
                             HyoutaUtils::EndianUtils::Endianness endianness =
                                 HyoutaUtils::EndianUtils::Endianness::LittleEndian) {
    for (size_t i = 0; i < dataLength; ++i) {
        s.WriteUInt16(data[i], endianness);
    }
}

ItemData::ItemData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    idx = stream.ReadUInt16();
    character = stream.ReadUInt16();
    flags = stream.ReadUTF8Nullterm();
    effect0 = ReadUInt16Array<7>(stream);
    effect1 = ReadUInt16Array<7>(stream);
    effect2 = ReadUInt16Array<7>(stream);
    effect3 = ReadUInt16Array<7>(stream);
    effect4 = ReadUInt16Array<7>(stream);
    effect5 = ReadUInt16Array<7>(stream);
    effect6 = ReadUInt16Array<7>(stream);
    stats = ReadUInt16Array<10>(stream);
    d1 = stream.ReadArray<9>();
    name = stream.ReadUTF8Nullterm();
    desc = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    d2.resize(dlen);
    stream.Read(d2.data(), dlen);
}

std::vector<char> ItemData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(idx);
        ms.WriteUInt16(character);
        ms.WriteUTF8Nullterm(flags);
        WriteUInt16Array(ms, effect0.data(), effect0.size());
        WriteUInt16Array(ms, effect1.data(), effect1.size());
        WriteUInt16Array(ms, effect2.data(), effect2.size());
        WriteUInt16Array(ms, effect3.data(), effect3.size());
        WriteUInt16Array(ms, effect4.data(), effect4.size());
        WriteUInt16Array(ms, effect5.data(), effect5.size());
        WriteUInt16Array(ms, effect6.data(), effect6.size());
        WriteUInt16Array(ms, stats.data(), stats.size());
        ms.Write(d1.data(), d1.size());
        ms.WriteUTF8Nullterm(name);
        ms.WriteUTF8Nullterm(desc);
        ms.Write(d2.data(), d2.size());
    }
    return rv;
}

ItemHelpData::ItemHelpData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    idx = stream.ReadUInt16();
    str = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    d.resize(dlen);
    stream.Read(d.data(), dlen);
}

std::vector<char> ItemHelpData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(idx);
        ms.WriteUTF8Nullterm(str);
        ms.Write(d.data(), d.size());
    }
    return rv;
}

CompHelpData::CompHelpData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    idx = stream.ReadUInt16();
    str = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    d.resize(dlen);
    stream.Read(d.data(), dlen);
}

std::vector<char> CompHelpData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(idx);
        ms.WriteUTF8Nullterm(str);
        ms.Write(d.data(), d.size());
    }
    return rv;
}

MagicData::MagicData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    idx = stream.ReadUInt16();
    d0 = stream.ReadUInt16();
    flags = stream.ReadUTF8Nullterm();
    d1 = stream.ReadArray<22>();
    for (size_t i = 0; i < effects.size(); ++i) {
        effects[i].idx = stream.ReadUInt16();
        for (size_t j = 0; j < effects[i].data.size(); ++j) {
            effects[i].data[j] = stream.ReadUInt32();
        }
    }
    d2 = stream.ReadArray<10>();
    animation = stream.ReadUTF8Nullterm();
    name = stream.ReadUTF8Nullterm();
    desc = stream.ReadUTF8Nullterm();

    assert(dataLength == stream.GetPosition());
}

std::vector<char> MagicData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(idx);
        ms.WriteUInt16(d0);
        ms.WriteUTF8Nullterm(flags);
        ms.Write(d1.data(), d1.size());
        for (size_t i = 0; i < effects.size(); ++i) {
            ms.WriteUInt16(effects[i].idx);
            for (size_t j = 0; j < effects[i].data.size(); ++j) {
                ms.WriteUInt32(effects[i].data[j]);
            }
        }
        ms.Write(d2.data(), d2.size());
        ms.WriteUTF8Nullterm(animation);
        ms.WriteUTF8Nullterm(name);
        ms.WriteUTF8Nullterm(desc);
    }
    return rv;
}

MonsterData::MonsterData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    file1 = stream.ReadUTF8Nullterm();
    file2 = stream.ReadUTF8Nullterm();
    file3 = stream.ReadUTF8Nullterm();
    d0 = stream.ReadArray<0xc5>();
    flags = stream.ReadUTF8Nullterm();
    name = stream.ReadUTF8Nullterm();
    desc = stream.ReadUTF8Nullterm();
}

std::vector<char> MonsterData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUTF8Nullterm(file1);
        ms.WriteUTF8Nullterm(file2);
        ms.WriteUTF8Nullterm(file3);
        ms.Write(d0.data(), d0.size());
        ms.WriteUTF8Nullterm(flags);
        ms.WriteUTF8Nullterm(name);
        ms.WriteUTF8Nullterm(desc);
    }
    return rv;
}

MasterQuartzMemo::MasterQuartzMemo(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    mqidx = stream.ReadUInt16();
    stridx = stream.ReadUInt16();
    str = stream.ReadUTF8Nullterm();
}

std::vector<char> MasterQuartzMemo::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(mqidx);
        ms.WriteUInt16(stridx);
        ms.WriteUTF8Nullterm(str);
    }
    return rv;
}

NameData::NameData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    idx = stream.ReadUInt16();
    name = stream.ReadUTF8Nullterm();
    str2 = stream.ReadUTF8Nullterm();
    str3 = stream.ReadUTF8Nullterm();
    str4 = stream.ReadUTF8Nullterm();
    str5 = stream.ReadUTF8Nullterm();
    str6 = stream.ReadUTF8Nullterm();
    const size_t dlen = dataLength - stream.GetPosition();
    d2.resize(dlen);
    stream.Read(d2.data(), dlen);
}

std::vector<char> NameData::ToBinary() const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream ms(rv);
        ms.WriteUInt16(idx);
        ms.WriteUTF8Nullterm(name);
        ms.WriteUTF8Nullterm(str2);
        ms.WriteUTF8Nullterm(str3);
        ms.WriteUTF8Nullterm(str4);
        ms.WriteUTF8Nullterm(str5);
        ms.WriteUTF8Nullterm(str6);
        ms.Write(d2.data(), d2.size());
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
    const size_t dlen = dataLength - stream.GetPosition();
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

VoiceTimingData::VoiceTimingData(const char* data,
                                 size_t dataLength,
                                 HyoutaUtils::EndianUtils::Endianness e,
                                 HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream s(data, dataLength);
    Index = s.ReadUInt16(e);
    const size_t dlen = dataLength - s.GetPosition();
    Unknown.resize(dlen);
    s.Read(Unknown.data(), dlen);
}

std::vector<char>
    VoiceTimingData::ToBinary(HyoutaUtils::EndianUtils::Endianness e,
                              HyoutaUtils::TextUtils::GameTextEncoding encoding) const {
    std::vector<char> rv;
    {
        HyoutaUtils::Stream::MemoryStream s(rv);
        s.WriteUInt16(Index, e);
        s.Write(Unknown.data(), Unknown.size());
    }
    return rv;
}
} // namespace SenLib::Sen3
