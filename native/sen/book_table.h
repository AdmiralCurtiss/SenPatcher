#pragma once

// warning: ported from C#, contains C#-isms

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib {
struct BookDataStruct {
    uint16_t Unknown1;
    std::array<char, 0x10> Name;
    uint16_t Unknown2;
    uint16_t Unknown3;
    uint16_t Unknown4;
    uint16_t Unknown5;
    uint16_t Unknown6;
    uint16_t Unknown7;
    uint16_t Unknown8;
    uint16_t Unknown9;
    uint16_t Unknown10;
    uint16_t Unknown11;

    BookDataStruct(ReadStream& s, HyoutaUtils::EndianUtils::Endianness e);
    void WriteToStream(WriteStream& s, HyoutaUtils::EndianUtils::Endianness e);
};

struct BookEntry {
    std::string Name;
    std::optional<uint16_t> Book99_Value1;
    std::optional<uint16_t> Book99_Value2;
    std::optional<std::vector<BookDataStruct>> BookDataStructs;
    std::optional<std::string> Text;
};

struct BookTable {
    std::string Name;
    uint32_t UnknownHeaderBytes;
    std::vector<BookEntry> Entries;

    BookTable(ReadStream& s,
              std::optional<HyoutaUtils::EndianUtils::Endianness> endian = std::nullopt,
              HyoutaUtils::TextUtils::GameTextEncoding encoding =
                  HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

    void WriteToStream(WriteStream& s, HyoutaUtils::EndianUtils::Endianness e);
};
} // namespace SenLib
