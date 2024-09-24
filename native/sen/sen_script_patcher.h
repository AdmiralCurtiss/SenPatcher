#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace SenLib {
struct SenScriptPatcher {
    static constexpr uint8_t JumpCommand = 0x03;
    static constexpr uint8_t NopCommand = 0x09;

    std::vector<char>& Bin;

    SenScriptPatcher(std::vector<char>& s) : Bin(s) {}
    SenScriptPatcher(const SenScriptPatcher& other) = delete;
    SenScriptPatcher(SenScriptPatcher&& other) = delete;
    SenScriptPatcher& operator=(const SenScriptPatcher& other) = delete;
    SenScriptPatcher& operator=(SenScriptPatcher&& other) = delete;
    ~SenScriptPatcher() = default;

    void ReplaceCommand(uint32_t originalLocation,
                        uint32_t originalLength,
                        std::span<const char> newCommand);

    void ReplacePartialCommand(uint32_t commandLocation,
                               uint32_t commandLength,
                               uint32_t replacementLocation,
                               uint32_t replacementLength,
                               std::span<const char> replacementData);

    void RemovePartialCommand(uint32_t commandLocation,
                              uint32_t commandLength,
                              uint32_t removeLocation,
                              uint32_t removeLength);

    void ExtendPartialCommand(uint32_t commandLocation,
                              uint32_t commandLength,
                              uint32_t extendLocation,
                              std::span<const char> extendData);

    // Moves data from [sourceLocation, sourceLocation + length) to targetLocation, shifting all
    // data in between to the side. Effectively cutting out some data and pasting it elsewhere.
    bool ShiftData(uint32_t sourceLocation, uint32_t targetLocation, uint32_t length);
};
} // namespace SenLib
