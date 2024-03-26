#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen2 {
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

    void WriteToStream(WriteStream& s,
                       HyoutaUtils::EndianUtils::Endianness e,
                       HyoutaUtils::TextUtils::GameTextEncoding encoding =
                           HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

private:
    uint16_t GetLength(const std::string& name,
                       ReadStream& stream,
                       HyoutaUtils::EndianUtils::Endianness e,
                       HyoutaUtils::TextUtils::GameTextEncoding encoding) const;
};

struct ItemData {
    uint16_t Idx;
    uint16_t Unknown0;
    std::string Flags;
    std::array<char, 0x3c> Unknown1;
    std::string Name;
    std::string Desc;
    std::array<char, 0x8> Unknown2;
    std::array<char, 0xc> Unknown3_QuartzOnly;
    bool IsQuartz;

    ItemData(const char* data, size_t dataLength, bool isQuartz);

    std::vector<char> ToBinary() const;
};

struct MagicData {
    uint16_t Idx;
    uint16_t Unknown0;
    std::string Flags;
    uint8_t Unknown1a;
    uint8_t Unknown1b;
    uint8_t Unknown1c;
    uint8_t Unknown1d;
    uint8_t Unknown1e;
    uint8_t Range;
    uint8_t Effect1_Type;    // 01 -> physical attack, 02 -> magic attack, 0x11 -> petrify chance,
                             // there's a ton of these...
    uint32_t Effect1_Value1; // depends on type; for attacks this is the attack power the Class is
                             // calculated from, for stat/ailments this is the % chance
    uint32_t Effect1_Value2; // depends on type; for stat/ailments this is the number of turns the
                             // thing is active
    uint8_t Unknown3a;
    uint8_t Unknown3b;
    uint8_t Unknown3c;
    uint8_t Unknown3d;
    uint8_t Effect2_Type;
    uint32_t Effect2_Value1;
    uint32_t Effect2_Value2;
    uint8_t Unknown5a;
    uint8_t Unknown5b;
    uint8_t Unknown5c;
    uint8_t Unknown5d;
    uint8_t Unknown5e;
    uint8_t Unknown6a;
    uint16_t Unknown6b;
    uint8_t Unbalance; // 0xff == no, otherwise % extra chance of unbalance compared to default
    uint8_t Unknown6d;
    uint16_t Unknown6e;
    std::string Anim;
    std::string Name;
    std::string Desc;

    MagicData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct CookData {
    std::string name;
    std::array<char, 0x22> d1;
    uint16_t item1;
    std::string item1line1;
    std::string item1line2;
    uint16_t item2;
    std::string item2line1;
    std::string item2line2;
    uint16_t item3;
    std::string item3line1;
    std::string item3line2;
    uint16_t item4;
    std::string item4line1;
    std::string item4line2;
    std::vector<char> d2;

    CookData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct VoiceData {
    uint16_t idx;
    std::string name;
    uint64_t unknown1;
    uint16_t unknown2;
    uint32_t unknown3;

    VoiceData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};
} // namespace SenLib::Sen2
