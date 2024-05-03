#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen1 {
struct TblEntry {
    std::string Name;
    std::vector<char> Data;
};

struct Tbl {
    std::vector<TblEntry> Entries;

    Tbl(const char* buffer,
        size_t length,
        HyoutaUtils::EndianUtils::Endianness e = HyoutaUtils::EndianUtils::Endianness::LittleEndian,
        HyoutaUtils::TextUtils::GameTextEncoding encoding =
            HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

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

struct ItemData {
    uint16_t Idx;
    uint16_t Usability;
    std::string Flags;
    uint8_t ItemType;
    uint8_t Element;
    uint8_t Status_Breaker;
    uint8_t Range;
    uint8_t AttackArea;
    uint8_t Action1_PassiveEffect;
    uint16_t Action1Value1_Art2;
    uint16_t Action1Value2_Art3;
    uint8_t Action2_Rarity;
    uint16_t Action2Value1_Art1;
    uint16_t Action2Value2_ProcChance;
    uint16_t STR;
    uint16_t DEF;
    uint16_t ATS;
    uint16_t ADF;
    uint16_t ACC;
    uint16_t EVA;
    uint16_t SPD;
    uint16_t MOV;
    uint16_t HP;
    uint16_t EP;
    uint32_t Mira;
    uint8_t CarryLimit;
    uint16_t SortOrder;
    uint16_t Unknown1;
    uint16_t Unknown2;
    std::string Name;
    std::string Desc;

    ItemData(const char* data,
             size_t dataLength,
             HyoutaUtils::EndianUtils::Endianness endian =
                 HyoutaUtils::EndianUtils::Endianness::LittleEndian,
             HyoutaUtils::TextUtils::GameTextEncoding encoding =
                 HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

    std::vector<char> ToBinary(HyoutaUtils::EndianUtils::Endianness endian =
                                   HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                               HyoutaUtils::TextUtils::GameTextEncoding encoding =
                                   HyoutaUtils::TextUtils::GameTextEncoding::UTF8) const;
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
    uint8_t Effect1_Type;
    uint16_t Effect1_Value1;
    uint16_t Effect1_Value2;
    uint8_t Effect2_Type;
    uint16_t Effect2_Value1;
    uint16_t Effect2_Value2;
    uint8_t Unknown5e;
    uint8_t Unknown6a;
    uint16_t Unknown6b;
    uint8_t Unbalance;
    uint8_t Unknown6d;
    uint16_t Unknown6e;
    std::string Anim;
    std::string Name;
    std::string Desc;

    MagicData(const char* data,
              size_t dataLength,
              HyoutaUtils::EndianUtils::Endianness endian =
                  HyoutaUtils::EndianUtils::Endianness::LittleEndian,
              HyoutaUtils::TextUtils::GameTextEncoding encoding =
                  HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

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
    uint16_t Index;
    std::string Name;
    uint64_t Unknown1;
    uint16_t Unknown2;
    uint32_t Unknown3;

    VoiceData(const char* data,
              size_t dataLength,
              HyoutaUtils::EndianUtils::Endianness e,
              HyoutaUtils::TextUtils::GameTextEncoding encoding =
                  HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

    std::vector<char> ToBinary(HyoutaUtils::EndianUtils::Endianness e,
                               HyoutaUtils::TextUtils::GameTextEncoding encoding =
                                   HyoutaUtils::TextUtils::GameTextEncoding::UTF8) const;
};
} // namespace SenLib::Sen1
