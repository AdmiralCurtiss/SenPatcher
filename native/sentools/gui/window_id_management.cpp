#include "window_id_management.h"

#include <bit>
#include <cassert>
#include <format>
#include <vector>

namespace SenTools::GUI {
// TODO: this is wasteful to store on the heap in the vast majority of cases,
// we can probably just store this locally in a fixed-size buffer and switch
// to heap in the unlikely case we exceed it...

struct WindowIdTracker::Impl {
    std::vector<bool> UsedWindowIds;
};

WindowIdTracker::WindowIdTracker() = default;

WindowIdTracker::~WindowIdTracker() {
    if (Storage != nullptr) {
        delete Storage;
    }
}

size_t WindowIdTracker::ReserveId() {
    if (Storage == nullptr) {
        Storage = new WindowIdTracker::Impl();
    }

    std::vector<bool>& usedWindowIds = Storage->UsedWindowIds;
    for (size_t i = 0; i < usedWindowIds.size(); ++i) {
        if (usedWindowIds[i] == false) {
            usedWindowIds[i] = true;
            return i;
        }
    }
    size_t nextId = usedWindowIds.size();
    usedWindowIds.push_back(true);
    return nextId;
}

void WindowIdTracker::ReturnId(size_t id) {
    assert(Storage != nullptr);
    if (Storage == nullptr) {
        return;
    }

    std::vector<bool>& usedWindowIds = Storage->UsedWindowIds;
    assert(id < usedWindowIds.size());
    if (id < usedWindowIds.size()) {
        assert(usedWindowIds[id] == true);
        usedWindowIds[id] = false;
    }
}

size_t GenerateWindowId(WindowIdTracker& tracker,
                        char* buffer,
                        size_t bufferLength,
                        const char* windowTitle,
                        size_t windowTitleSizeOf) {
    assert(bufferLength >= GetWindowIdBufferLength(windowTitleSizeOf));

    const size_t newWindowId = tracker.ReserveId();
    auto result = std::format_to_n(buffer, bufferLength - 1, "{}##W{:x}", windowTitle, newWindowId);
    *result.out = '\0';
    return newWindowId;
}

} // namespace SenTools::GUI
