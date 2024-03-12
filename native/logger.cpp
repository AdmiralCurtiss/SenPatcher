#include "logger.h"

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace SenPatcher {
Logger::Logger(const char* filename) {
    if (filename) {
        f = fopen(filename, "w");
    } else {
        f = nullptr;
    }
}

Logger::~Logger() {
    if (f) {
        fclose(f);
    }
}

Logger& Logger::Log(std::string_view text) {
    if (f) {
        fwrite(text.data(), 1, text.size(), f);
        fflush(f);
    }
    return *this;
}

Logger& Logger::LogPtr(const void* ptr) {
    if (f) {
        char buffer[32];
        int len = sprintf(buffer, "0x%016" PRIxPTR, reinterpret_cast<uintptr_t>(ptr));
        fwrite(buffer, len, 1, f);
        fflush(f);
    }
    return *this;
}

Logger& Logger::LogInt(int v) {
    if (f) {
        char buffer[32];
        int len = sprintf(buffer, "%d", v);
        fwrite(buffer, len, 1, f);
        fflush(f);
    }
    return *this;
}

Logger& Logger::LogHex(unsigned long long v) {
    if (f) {
        char buffer[32];
        int len = sprintf(buffer, "0x%llx", v);
        fwrite(buffer, len, 1, f);
        fflush(f);
    }
    return *this;
}

Logger& Logger::LogFloat(float v) {
    if (f) {
        char buffer[32];
        int len = sprintf(buffer, "%g", v);
        fwrite(buffer, len, 1, f);
        fflush(f);
    }
    return *this;
}
} // namespace SenPatcher
