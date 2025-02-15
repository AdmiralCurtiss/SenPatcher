#pragma once

#include <cstddef>

namespace SenTools::GUI {
struct WindowIdTracker {
    WindowIdTracker();
    WindowIdTracker(const WindowIdTracker& other) = delete;
    WindowIdTracker(WindowIdTracker&& other) = delete;
    WindowIdTracker& operator=(const WindowIdTracker& other) = delete;
    WindowIdTracker& operator=(WindowIdTracker&& other) = delete;
    ~WindowIdTracker();

    size_t ReserveId();
    void ReturnId(size_t id);

private:
    struct Impl;
    Impl* Storage = nullptr;
};

inline constexpr size_t GetWindowIdBufferLength(size_t windowTitleSizeOf) {
    // size is: actual title (including null) + "##W" + index converted to hex
    return windowTitleSizeOf + 3 + sizeof(size_t) * 2;
}

size_t GenerateWindowId(WindowIdTracker& tracker,
                        char* buffer,
                        size_t bufferLength,
                        const char* windowTitle,
                        size_t windowTitleSizeOf);
} // namespace SenTools::GUI
