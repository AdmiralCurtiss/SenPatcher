#pragma once

#include <cstdint>
#include <string_view>

#include "util/file.h"

namespace HyoutaUtils {
struct Logger {
    explicit Logger();
    explicit Logger(HyoutaUtils::IO::File file);
    Logger(const Logger& other) = delete;
    Logger(Logger&& other);
    Logger& operator=(const Logger& other) = delete;
    Logger& operator=(Logger&& other);
    ~Logger();

    Logger& Log(std::string_view text);
    Logger& LogPtr(const void* ptr);
    Logger& LogInt(int v);
    Logger& LogHex(unsigned long long v);
    Logger& LogFloat(float v);

private:
    HyoutaUtils::IO::File Handle;
};
} // namespace HyoutaUtils
