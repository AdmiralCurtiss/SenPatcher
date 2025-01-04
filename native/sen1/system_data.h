#pragma once

#include <array>
#include <cstdint>

#include "util/endian.h"

namespace SenLib::Sen1 {
struct SystemData {
    static constexpr size_t FileLength = 52;

    bool Deserialize(const char* buffer,
                     size_t length,
                     HyoutaUtils::EndianUtils::Endianness e =
                         HyoutaUtils::EndianUtils::Endianness::LittleEndian);

    std::array<char, FileLength>
        Serialize(HyoutaUtils::EndianUtils::Endianness e =
                      HyoutaUtils::EndianUtils::Endianness::LittleEndian) const;

    // possibly a game identifier, as this is the sixth mainline Kiseki game?
    uint32_t Always6 = 0;

    uint8_t Unknown1 = 0;

    // this value clearly means something but I'm not sure what
    uint8_t Unknown2 = 0;

    // 0x00 - 0x15 ish, not all of them seem used
    uint8_t TitleScreenVariant = 0;

    uint8_t Unknown4 = 0;

    uint8_t Unknown5 = 0;

    uint8_t Unknown6 = 0;

    uint8_t Unknown7 = 0;

    uint8_t Unknown8 = 0;

    uint8_t Unknown9 = 0;

    uint8_t Unknown10 = 0;

    uint8_t Unknown11 = 0;

    uint8_t Unknown12 = 0;

    // 0x0 to 0xA
    uint16_t BgmVolume = 0;
    uint16_t SeVolume = 0;
    uint16_t VoiceVolume = 0;

    // 0 == normal, 1 == inverted
    uint16_t VerticalCamera = 0;
    uint16_t HorizontalCamera = 0;

    // 0 == main notebook
    // 1 == quest notebook
    // 2 == battle notebook
    // 3 == character notebook
    // 4 == recipe book
    // 5 == fishing book
    // 6 == books
    // 7 == active voice notes
    // 8 == help
    uint16_t L2ButtonMapping = 0;
    uint16_t R2ButtonMapping = 0;

    // 0 == Zoom In
    // 1 == Zoom Out
    // 2 == Zoom In/Out
    // 3 == Reset Camera
    // 4 == Turn Character
    // 5 == Change Leader (Next)
    // 6 == Change Leader (Previous)
    uint16_t DPadUpButtonMapping = 0;
    uint16_t DPadDownButtonMapping = 0;
    uint16_t DPadLeftButtonMapping = 0;
    uint16_t DPadRightButtonMapping = 0;
    uint16_t L1ButtonMapping; // Turn Character not allowed

    // 0 == rotate, 1 == fixed
    uint16_t Minimap = 0;

    // 0 == on, 1 == off
    uint16_t Vibration = 0;

    // from 0 to 100 (0x64) in increments of 2; default 100, which fills whole screen
    // in PC version this is generates artifacts so probably should be left alone
    uint16_t ScreenWidth = 0;
    uint16_t ScreenHeight = 0;

    // from 0 to 100 (0x64) in increments of 2; default 50 (0x32), less is darker, more is brighter
    uint16_t ScreenBrightness = 0;

    uint16_t Unused = 0;
};
} // namespace SenLib::Sen1
