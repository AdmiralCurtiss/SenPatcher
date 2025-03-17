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

namespace SenLib::Sen4 {
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
        // some of these have incorrect length fields in the official files on PS4, manually
        // determine length here...
        stream.DiscardBytes(2);
        uint64_t p = stream.GetPosition();
        stream.DiscardBytes(3);
        stream.ReadNulltermString(encoding);
        stream.ReadNulltermString(encoding);
        stream.DiscardBytes(13);
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

ItemData::ItemData(const char* data, size_t dataLength) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data, dataLength);
    idx = stream.ReadUInt16();
    character = stream.ReadUInt16();
    flags = stream.ReadUTF8Nullterm();
    d1 = stream.ReadArray<0x96>();
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
        ms.Write(d1.data(), d1.size());
        ms.WriteUTF8Nullterm(name);
        ms.WriteUTF8Nullterm(desc);
        ms.Write(d2.data(), d2.size());
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
    d2 = stream.ReadArray<11>();
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
} // namespace SenLib::Sen4
