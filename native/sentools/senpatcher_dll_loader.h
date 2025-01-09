#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "util/file.h"

namespace SenTools {
enum class SenPatcherDllIdentificationType {
    UnspecifiedError,
    FileDoesNotExist,
    FileInaccessible,
    NotASenPatcherDll,
    CS1Hook,
    CS2Hook,
    CS3Hook,
    CS4Hook,
    ReverieHook,
    TXHook,

    // has valid SenPatcher data but is not any of the ones we recognize, maybe a game that we added
    // support to after this version was built
    UnknownHook,
};

struct SenPatcherDllIdentificationResult {
    SenPatcherDllIdentificationType Type = SenPatcherDllIdentificationType::UnspecifiedError;
    std::optional<std::string> Version;
    std::optional<std::string> FileFixInfo;
};

SenPatcherDllIdentificationResult IdentifySenPatcherDll(std::string_view path);
SenPatcherDllIdentificationResult IdentifySenPatcherDll(HyoutaUtils::IO::File& file);
} // namespace SenTools
