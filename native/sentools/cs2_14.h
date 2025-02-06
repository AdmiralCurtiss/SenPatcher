#pragma once

#include <functional>
#include <string_view>

namespace SenTools {
enum class TryPatchCs2Version14Result {
    IsNotV14,
    UpdateDeclined,
    PatchingFailed,
    WritingNewFilesFailed,
    UpdateSucceeded,
};

// This attempts to patch CS2 v1.4 to v1.4.2
// We do this because the Humble Bundle DRM-free download of CS2 was never updated to 1.4.1
// or 1.4.2, and the binaries of CS2 1.4 and 1.4.1/1.4.2 are different enough (different MSVC
// compiler version) that supporting both would be a huge pain.
// This returns true if we can proceed with the SenPatcher installation, or false if not; more
// specifically:
// - true if we don't detect CS2 1.4
// - false if the update was declined via the callback
// - false if the update failed
// - true if the update succeeded
TryPatchCs2Version14Result TryPatchCs2Version14(std::string_view path,
                                                const std::function<bool()>& confirmationCallback);
} // namespace SenTools
