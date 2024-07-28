#include "logger.h"

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "util/file.h"

namespace HyoutaUtils {
Logger::Logger() = default;
Logger::Logger(HyoutaUtils::IO::File file) : Handle(std::move(file)) {}
Logger::Logger(Logger&& other) = default;
Logger& Logger::operator=(Logger&& other) = default;

Logger::~Logger() = default;

Logger& Logger::Log(std::string_view text) {
    if (Handle.IsOpen()) {
        Handle.Write(text.data(), text.size());
    }
    return *this;
}

Logger& Logger::LogPtr(const void* ptr) {
    if (Handle.IsOpen()) {
        char buffer[32];
        int len = sprintf(buffer, "0x%016" PRIxPTR, reinterpret_cast<uintptr_t>(ptr));
        Handle.Write(buffer, len);
    }
    return *this;
}

Logger& Logger::LogInt(int v) {
    if (Handle.IsOpen()) {
        char buffer[32];
        int len = sprintf(buffer, "%d", v);
        Handle.Write(buffer, len);
    }
    return *this;
}

Logger& Logger::LogHex(unsigned long long v) {
    if (Handle.IsOpen()) {
        char buffer[32];
        int len = sprintf(buffer, "0x%llx", v);
        Handle.Write(buffer, len);
    }
    return *this;
}

Logger& Logger::LogFloat(float v) {
    if (Handle.IsOpen()) {
        char buffer[32];
        int len = sprintf(buffer, "%g", v);
        Handle.Write(buffer, len);
    }
    return *this;
}
} // namespace HyoutaUtils
