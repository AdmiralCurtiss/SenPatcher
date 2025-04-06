#pragma once

#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class SaveChecksumFixResult { Success, AlreadyCorrect };
enum class SaveFileType {
    Unknown, // will attempt to autodetect
    CS4_SystemData,
    CS4_GameData,
    Reverie_SystemData,
    Reverie_GameData,
    Daybreak_GameData,
    Daybreak2_GameData,
};

HyoutaUtils::Result<SaveChecksumFixResult, std::string>
    SaveChecksumFix(std::string_view source,
                    std::string_view target,
                    SaveFileType saveFileType = SaveFileType::Unknown,
                    bool forceCompress = false,
                    bool forceWrite = false);
HyoutaUtils::Result<SaveChecksumFixResult, std::string>
    SaveChecksumFix(char* saveMemoryData,
                    size_t saveMemoryLength,
                    std::string_view target,
                    SaveFileType saveFileType = SaveFileType::Unknown,
                    bool forceCompress = false,
                    bool forceWrite = false);
} // namespace SenTools
