#include "book_table.h"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

// warning: ported from C#, contains C#-isms

namespace SenLib {
BookDataStruct::BookDataStruct(HyoutaUtils::Stream::ReadStream& s,
                               HyoutaUtils::EndianUtils::Endianness e) {
    Unknown1 = s.ReadUInt16(e);
    Name = s.ReadArray<0x10>();
    Unknown2 = s.ReadUInt16(e);
    Unknown3 = s.ReadUInt16(e);
    Unknown4 = s.ReadUInt16(e);
    Unknown5 = s.ReadUInt16(e);
    Unknown6 = s.ReadUInt16(e);
    Unknown7 = s.ReadUInt16(e);
    Unknown8 = s.ReadUInt16(e);
    Unknown9 = s.ReadUInt16(e);
    Unknown10 = s.ReadUInt16(e);
    Unknown11 = s.ReadUInt16(e);
}

void BookDataStruct::WriteToStream(HyoutaUtils::Stream::WriteStream& s,
                                   HyoutaUtils::EndianUtils::Endianness e) {
    s.WriteUInt16(Unknown1, e);
    s.WriteArray(Name);
    s.WriteUInt16(Unknown2, e);
    s.WriteUInt16(Unknown3, e);
    s.WriteUInt16(Unknown4, e);
    s.WriteUInt16(Unknown5, e);
    s.WriteUInt16(Unknown6, e);
    s.WriteUInt16(Unknown7, e);
    s.WriteUInt16(Unknown8, e);
    s.WriteUInt16(Unknown9, e);
    s.WriteUInt16(Unknown10, e);
    s.WriteUInt16(Unknown11, e);
}

BookTable::BookTable(HyoutaUtils::Stream::ReadStream& s,
                     std::optional<HyoutaUtils::EndianUtils::Endianness> endian,
                     HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    uint32_t headerLengthLittleEndian =
        s.ReadUInt32(HyoutaUtils::EndianUtils::Endianness::LittleEndian);

    const HyoutaUtils::EndianUtils::Endianness e =
        endian
            ? *endian
            : (headerLengthLittleEndian == 0x20 ? HyoutaUtils::EndianUtils::Endianness::LittleEndian
                                                : HyoutaUtils::EndianUtils::Endianness::BigEndian);
    uint32_t headerLength = e == HyoutaUtils::EndianUtils::Endianness::LittleEndian
                                ? headerLengthLittleEndian
                                : HyoutaUtils::EndianUtils::SwapEndian(headerLengthLittleEndian);
    if (headerLength != 0x20) {
        throw "unexpected header length";
    }

    uint32_t namePosition = s.ReadUInt32(e);
    uint32_t functionOffsetsPosition = s.ReadUInt32(e);
    uint32_t functionOffsetsLength = s.ReadUInt32(e);
    uint32_t functionNameOffsetsPosition = s.ReadUInt32(e);
    uint32_t functionCount = s.ReadUInt32(e); // seems redundant between length and count...?
    if (functionCount * 4 != functionOffsetsLength) {
        throw "inconsistency"; // maybe one of the two means something else then?
    }
    uint32_t functionMetadataEnd = s.ReadUInt32(e);
    UnknownHeaderBytes = s.ReadUInt32(e);

    Name = s.ReadAsciiNulltermFromLocationAndReset(namePosition);
    s.SetPosition(functionOffsetsPosition);
    auto functionPositions = std::make_unique_for_overwrite<uint32_t[]>(functionCount);
    for (size_t i = 0; i < functionCount; ++i) {
        functionPositions[i] = s.ReadUInt32(e);
    }
    s.SetPosition(functionNameOffsetsPosition);
    auto functionNamePositions = std::make_unique_for_overwrite<uint16_t[]>(functionCount);
    for (size_t i = 0; i < functionCount; ++i) {
        functionNamePositions[i] = s.ReadUInt16(e);
    }
    auto functionNames = std::make_unique<std::string[]>(functionCount);
    for (int64_t i = 0; i < functionCount; ++i) {
        s.SetPosition(functionNamePositions[i]);
        functionNames[i] = s.ReadAsciiNullterm();
    }

    std::vector<BookEntry> funcs;
    for (int64_t i = 0; i < functionCount; ++i) {
        bool is99 = functionNames[i].ends_with("_99");
        if (is99) {
            s.SetPosition(functionPositions[i]);
            uint16_t d1 = s.ReadUInt16(e);
            uint16_t d2 = s.ReadUInt16(e);
            uint8_t terminator = s.ReadUInt8();
            if (terminator != 0x01) {
                throw "unexpected format";
            }
            funcs.emplace_back(
                BookEntry{.Name = functionNames[i], .Book99_Value1 = d1, .Book99_Value2 = d2});
        } else {
            s.SetPosition(functionPositions[i]);
            uint16_t dataCounter = s.ReadUInt16(e);
            std::vector<BookDataStruct> data;
            data.reserve(dataCounter);
            for (int j = 0; j < dataCounter; ++j) {
                data.push_back(BookDataStruct(s, e));
            }
            std::string text = s.ReadNulltermString(encoding);
            uint8_t terminator = s.ReadUInt8();
            if (terminator != 0x01) {
                throw "unexpected format";
            }
            funcs.emplace_back(
                BookEntry{.Name = functionNames[i], .BookDataStructs = data, .Text = text});
        }
    }

    Entries = funcs;
}

void BookTable::WriteToStream(HyoutaUtils::Stream::WriteStream& s,
                              HyoutaUtils::EndianUtils::Endianness e) {
    s.Clear();

    // header, fill in once we know the offsets
    for (int i = 0; i < 8; ++i) {
        s.WriteUInt32(0, e);
    }

    uint32_t namePosition = (uint32_t)s.GetPosition();
    s.WriteAsciiNullterm(Name);
    uint32_t functionOffsetsPosition = (uint32_t)s.GetPosition();
    uint32_t functionOffsetsLength = ((uint32_t)Entries.size()) * 4u;
    uint32_t functionNameOffsetsPosition = functionOffsetsPosition + functionOffsetsLength;
    uint32_t functionCount = (uint32_t)Entries.size();
    uint32_t unknown = UnknownHeaderBytes;

    // offsets to functions, fill later
    for (int i = 0; i < Entries.size(); ++i) {
        s.WriteUInt32(0, e);
    }
    // offsets to names of functions, fill later
    for (int i = 0; i < Entries.size(); ++i) {
        s.WriteUInt16(0, e);
    }

    // write function names + offsets
    for (int i = 0; i < Entries.size(); ++i) {
        int64_t o = s.GetPosition();
        s.WriteAsciiNullterm(Entries[i].Name);
        int64_t p = s.GetPosition();
        s.SetPosition(functionOffsetsPosition + functionOffsetsLength + i * 2);
        s.WriteUInt16((uint16_t)o, e);
        s.SetPosition(p);
    }

    uint32_t functionMetadataEnd = (uint32_t)s.GetPosition();

    // write functions
    for (int i = 0; i < Entries.size(); ++i) {
        s.WriteAlign(4); // probably?

        int64_t o = s.GetPosition();

        if (Entries[i].Book99_Value1.has_value()) {
            s.WriteUInt16(*(Entries[i].Book99_Value1), e);
        }
        if (Entries[i].Book99_Value2.has_value()) {
            s.WriteUInt16(*(Entries[i].Book99_Value2), e);
        }
        if (Entries[i].BookDataStructs.has_value()) {
            s.WriteUInt16((uint16_t)Entries[i].BookDataStructs->size(), e);
            for (int j = 0; j < Entries[i].BookDataStructs->size(); ++j) {
                (*Entries[i].BookDataStructs)[j].WriteToStream(s, e);
            }
        }
        if (Entries[i].Text.has_value()) {
            s.WriteUTF8Nullterm(*(Entries[i].Text));
        }
        s.WriteUInt8(0x01);

        int64_t p = s.GetPosition();
        s.SetPosition(functionOffsetsPosition + i * 4);
        s.WriteUInt32((uint32_t)o, e);
        s.SetPosition(p);
    }

    // write header
    s.SetPosition(0);
    s.WriteUInt32(0x20, e);
    s.WriteUInt32(namePosition, e);
    s.WriteUInt32(functionOffsetsPosition, e);
    s.WriteUInt32(functionOffsetsLength, e);
    s.WriteUInt32(functionNameOffsetsPosition, e);
    s.WriteUInt32(functionCount, e);
    s.WriteUInt32(functionMetadataEnd, e);
    s.WriteUInt32(unknown, e);

    s.SetPosition(0);
}
} // namespace SenLib
