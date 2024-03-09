#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace SenLib {
struct SenScriptPatcher {
    static constexpr uint8_t JumpCommand = 0x03;
    static constexpr uint8_t NopCommand = 0x09;

    std::vector<char>& Bin;

    SenScriptPatcher(std::vector<char>& s);

    void ReplaceCommand(int64_t originalLocation,
                        int64_t originalLength,
                        std::span<const char> newCommand);

    void ReplacePartialCommand(int64_t commandLocation,
                               int64_t commandLength,
                               int64_t replacementLocation,
                               int64_t replacementLength,
                               std::span<const char> replacementData);

    void RemovePartialCommand(int64_t commandLocation,
                              int64_t commandLength,
                              int64_t removeLocation,
                              int64_t removeLength);

    void ExtendPartialCommand(int64_t commandLocation,
                              int64_t commandLength,
                              int64_t extendLocation,
                              std::span<const char> extendData);
};
} // namespace SenLib
