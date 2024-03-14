#pragma once

#include <cstdint>
#include <string_view>

#include "file.h"

namespace SenPatcher {
struct Logger {
    explicit Logger();
    explicit Logger(SenPatcher::IO::File file);
    Logger(const Logger& other) = delete;
    Logger(Logger&& other) = delete;
    Logger& operator=(const Logger& other) = delete;
    Logger& operator=(Logger&& other) = delete;
    ~Logger();

    Logger& Log(std::string_view text);
    Logger& LogPtr(const void* ptr);
    Logger& LogInt(int v);
    Logger& LogHex(unsigned long long v);
    Logger& LogFloat(float v);

private:
    SenPatcher::IO::File Handle;
};
} // namespace SenPatcher
