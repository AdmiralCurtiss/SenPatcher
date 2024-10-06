#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::TX {
struct TblDefinition {
    std::string Name;
    uint32_t NumberOfEntries;
};

struct TblEntry {
    std::string Name;
    std::vector<char> Data;
};

struct Tbl {
    std::vector<TblDefinition> Definitions;
    std::vector<TblEntry> Entries;

    Tbl(const char* buffer,
        size_t length,
        HyoutaUtils::EndianUtils::Endianness e = HyoutaUtils::EndianUtils::Endianness::LittleEndian,
        HyoutaUtils::TextUtils::GameTextEncoding encoding =
            HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

    void RecalcNumberOfEntries();

    void WriteToStream(HyoutaUtils::Stream::WriteStream& s,
                       HyoutaUtils::EndianUtils::Endianness e,
                       HyoutaUtils::TextUtils::GameTextEncoding encoding =
                           HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

private:
    uint16_t GetLength(const std::string& name,
                       HyoutaUtils::Stream::ReadStream& stream,
                       HyoutaUtils::EndianUtils::Endianness e,
                       HyoutaUtils::TextUtils::GameTextEncoding encoding) const;
};

struct TextTableData {
    uint16_t Idx;
    std::string Str;

    TextTableData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct ItemHelpData {
    uint16_t Idx;
    std::string Str;
    std::array<char, 9> D;

    ItemHelpData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct hkitugi_lst {
    uint16_t Idx;
    uint16_t Unknown1;
    uint16_t Unknown2;
    std::string Name;
    std::string Description;

    hkitugi_lst(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct ActiveVoiceTableData {
    uint16_t Idx;
    uint16_t Unknown1;
    uint16_t Unknown2;
    std::string Image;
    uint16_t Unknown3;
    uint16_t Unknown4;
    std::string Text;
    uint32_t Unknown5;
    uint32_t Unknown6;
    uint16_t Unknown7;
    uint16_t Unknown8;
    uint16_t Unknown9;
    uint16_t Unknown10;
    uint16_t Unknown11;

    ActiveVoiceTableData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct DungeonData {
    uint16_t Idx;
    uint16_t Unknown1;
    uint8_t Unknown2;
    std::string Name;
    std::vector<char> D;

    DungeonData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct ItemData {
    uint16_t Idx;
    uint16_t Unknown1;
    std::string Flags;
    std::array<char, 0x3e> Unknown2;
    std::string Name;
    std::string Description;
    std::vector<char> D;

    ItemData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct MapJumpData {
    uint16_t Idx;
    uint16_t Unknown1;
    std::string Name;
    std::vector<char> D;

    MapJumpData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct MagicData {
    uint16_t Idx;
    uint16_t Unknown1;
    std::string Flags;
    std::array<char, 0x33> Unknown2;
    std::string Animation;
    std::string Name;
    std::string Description;

    MagicData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct QSText {
    uint16_t Idx;
    uint8_t Unknown1;
    std::string Text;
    std::vector<char> D;

    QSText(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct QSChar {
    uint16_t Idx;
    uint16_t Unknown1;
    uint16_t Unknown2;
    uint8_t Unknown3;
    std::array<std::string, 15> Strings;

    QSChar(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct QSStage {
    uint16_t Idx;
    uint16_t Unknown1;
    std::string Name;
    std::vector<char> D;

    QSStage(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct PlaceTableData {
    uint16_t Idx;
    uint16_t Unknown1;
    std::string Map;
    std::string Name;
    std::vector<char> D;

    PlaceTableData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

} // namespace SenLib::TX
