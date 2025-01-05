#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "util/endian.h"

namespace SenLib::Sen2 {
struct SystemData {
    static constexpr size_t FileLength = 120;

    bool Deserialize(const char* buffer,
                     size_t length,
                     HyoutaUtils::EndianUtils::Endianness e =
                         HyoutaUtils::EndianUtils::Endianness::LittleEndian);

    std::array<char, FileLength>
        Serialize(HyoutaUtils::EndianUtils::Endianness e =
                      HyoutaUtils::EndianUtils::Endianness::LittleEndian) const;

    // possibly a game identifier, as this is the seventh mainline Kiseki game?
    uint32_t Always7;

    uint8_t Unknown1;

    // this value clearly means something but I'm not sure what
    uint8_t Unknown2;

    // 0x0 - 0xD
    uint8_t TitleScreenVariant;

    uint8_t Unknown4;

    // bitfield for the Memories section in the main menu
    // note: if a character's Ending flag is set, the dorm event is also unlocked implicitly
    // bit 0 = Memories menu, clear locked, set unlocked
    // bit 1 = Alisa Ending, clear locked, set unlocked
    // bit 2 = Alisa Dorm Event, clear locked, set unlocked
    // bit 3 = Elliot Ending, clear locked, set unlocked
    // bit 4 = Elliot Dorm Event, clear locked, set unlocked
    // bit 5 = Laura Ending, clear locked, set unlocked
    // bit 6 = Laura Dorm Event, clear locked, set unlocked
    // bit 7 = Machias Ending, clear locked, set unlocked
    // bit 8 = Machias Dorm Event, clear locked, set unlocked
    // bit 9 = Emma Ending, clear locked, set unlocked
    // bit 10 = Emma Dorm Event, clear locked, set unlocked
    // bit 11 = Jusis Ending, clear locked, set unlocked
    // bit 12 = Jusis Dorm Event, clear locked, set unlocked
    // bit 13 = Fie Ending, clear locked, set unlocked
    // bit 14 = Fie Dorm Event, clear locked, set unlocked
    // bit 15 = Gaius Ending, clear locked, set unlocked
    // bit 16 = Gaius Dorm Event, clear locked, set unlocked
    // bit 17 = Millium Ending, clear locked, set unlocked
    // bit 18 = Millium Dorm Event, clear locked, set unlocked
    // bit 18 = Sara Ending, clear locked, set unlocked
    // bit 20 = Sara Dorm Event, clear locked, set unlocked
    // bit 21 = Towa Ending, clear locked, set unlocked
    // bit 22 = Towa Dorm Event, clear locked, set unlocked
    // bit 23 = Alfin Ending, clear locked, set unlocked
    // bit 24 = Alfin Dorm Event, clear locked, set unlocked
    // bit 25 = Toval Farewell, clear locked, set unlocked
    // bit 26 = Claire Farewell, clear locked, set unlocked
    // bit 27 = Sharon Farewell, clear locked, set unlocked
    // bits 28-31 seem unused
    uint32_t MemoriesBitfield;

    uint8_t Unknown9;

    uint8_t Unknown10;

    uint8_t Unknown11;

    uint8_t Unknown12;

    // 0x0 to 0xA
    uint16_t BgmVolume;
    uint16_t SeVolume;
    uint16_t VoiceVolume;

    // 0 == normal, 1 == inverted
    uint16_t VerticalCamera;
    uint16_t HorizontalCamera;

    // 0 == main notebook
    // 1 == quest notebook
    // 2 == battle notebook
    // 3 == character notebook
    // 4 == recipe book
    // 5 == fishing book
    // 6 == books
    // 7 == active voice notes
    // 8 == help
    uint16_t L2ButtonMapping;
    uint16_t R2ButtonMapping;

    // 0 == Zoom In
    // 1 == Zoom Out
    // 2 == Zoom In/Out
    // 3 == Reset Camera
    // 4 == Turn Character
    // 5 == Change Leader (Next)
    // 6 == Change Leader (Previous)
    // 7 == Dash
    // 8 == Walk
    uint16_t DPadUpButtonMapping;    // Dash/Walk not allowed
    uint16_t DPadDownButtonMapping;  // Dash/Walk not allowed
    uint16_t DPadLeftButtonMapping;  // Dash/Walk not allowed
    uint16_t DPadRightButtonMapping; // Dash/Walk not allowed
    uint16_t L1ButtonMapping;        // Turn Character not allowed
    uint16_t R1ButtonMapping;
    uint16_t CircleButtonMapping; // Only Dash/Walk allowed

    // 0 == rotate, 1 == fixed
    uint16_t Minimap;

    // 0 == on, 1 == off
    uint16_t Vibration;

    // from 0 to 100 (0x64) in increments of 2; default 100, which fills whole screen
    // in PC version this is generates artifacts so probably should be left alone
    uint16_t ScreenWidth;
    uint16_t ScreenHeight;

    // from 0 to 100 (0x64) in increments of 2; default 50 (0x32), less is darker, more is brighter
    uint16_t ScreenBrightness;

    // 0 == wait, 1 == free
    uint16_t ActiveVoice;

    uint64_t Unknown13;
    uint64_t Unknown14;
    uint64_t Unknown15;
    uint64_t Unknown16;
    uint64_t Unknown17;
    uint64_t Unknown18;
    uint64_t Unknown19;
    uint64_t Unknown20;
};
} // namespace SenLib::Sen2
