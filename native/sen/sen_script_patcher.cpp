#include "sen_script_patcher.h"

#include <cstdint>
#include <span>
#include <vector>

#include "../util/stream.h"

namespace SenLib {
SenScriptPatcher::SenScriptPatcher(std::vector<char>& s) : Bin(s) {}

void SenScriptPatcher::ReplaceCommand(int64_t originalLocation,
                                      int64_t originalLength,
                                      std::span<const char> newCommand) {
    int64_t nextCommandLocation = originalLocation + originalLength;

    MemoryStream bin(Bin);

    if (newCommand.size() <= originalLength) {
        // this is simple, just put the new command and fill the rest with NOP
        bin.CurrentPosition = originalLocation;
        bin.Write(newCommand.data(), newCommand.size());
        int64_t rest = nextCommandLocation - bin.CurrentPosition;
        for (int64_t i = 0; i < rest; ++i) {
            bin.WriteByte(NopCommand);
        }
        Bin = std::move(bin.Data);
        return;
    }

    // new command is longer than the old one
    // need to write a jump, write the new command in free space at the bottom of the file, and then
    // jump back

    if (originalLength < 5) {
        Bin = std::move(bin.Data);
        throw "Not enough space to write jump command.";
    }

    // write new command at and of file
    bin.CurrentPosition = bin.Data.size();
    bin.WriteAlign(4);
    uint32_t newCommandLocation = (uint32_t)bin.CurrentPosition;
    bin.Write(newCommand.data(), newCommand.size());

    // then a jump back to the actual script
    bin.WriteUInt8(JumpCommand);
    bin.WriteUInt32((uint32_t)nextCommandLocation,
                    HyoutaUtils::EndianUtils::Endianness::LittleEndian);

    // jump to the new command back at the original command
    bin.CurrentPosition = originalLocation;
    bin.WriteUInt8(JumpCommand);
    bin.WriteUInt32(newCommandLocation, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

    // and to be clean, dummy out the rest of the old command
    for (int64_t i = 0; i < originalLength - 5; ++i) {
        bin.WriteUInt8(NopCommand);
    }

    Bin = std::move(bin.Data);
}

void SenScriptPatcher::ReplacePartialCommand(int64_t commandLocation,
                                             int64_t commandLength,
                                             int64_t replacementLocation,
                                             int64_t replacementLength,
                                             std::span<const char> replacementData) {
    int64_t keepByteCountStart = replacementLocation - commandLocation;
    int64_t keepByteCountEnd =
        (commandLocation + commandLength) - (replacementLocation + replacementLength);

    std::vector<char> newCommand;
    if (keepByteCountStart > 0) {
        auto begin = Bin.begin() + commandLocation;
        newCommand.insert(newCommand.end(), begin, begin + keepByteCountStart);
    }
    if (replacementData.size() > 0) {
        newCommand.insert(newCommand.end(), replacementData.begin(), replacementData.end());
    }
    if (keepByteCountEnd > 0) {
        auto begin = Bin.begin() + (commandLocation + commandLength) - keepByteCountEnd;
        newCommand.insert(newCommand.end(), begin, begin + keepByteCountEnd);
    }

    ReplaceCommand(commandLocation, commandLength, newCommand);
}

void SenScriptPatcher::RemovePartialCommand(int64_t commandLocation,
                                            int64_t commandLength,
                                            int64_t removeLocation,
                                            int64_t removeLength) {
    ReplacePartialCommand(
        commandLocation, commandLength, removeLocation, removeLength, std::span<const char>());
}

void SenScriptPatcher::ExtendPartialCommand(int64_t commandLocation,
                                            int64_t commandLength,
                                            int64_t extendLocation,
                                            std::span<const char> extendData) {
    ReplacePartialCommand(commandLocation, commandLength, extendLocation, 0, extendData);
}
} // namespace SenLib
