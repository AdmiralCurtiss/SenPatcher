#pragma once

#include <cstdio>

namespace SenPatcher {
struct Logger {
private:
    FILE* f; // TODO: change this to SenPatcher::File

public:
    explicit Logger(const char* filename);
    Logger(const Logger& other) = delete;
    Logger(Logger&& other) = delete;
    Logger& operator=(const Logger& other) = delete;
    Logger& operator=(Logger&& other) = delete;
    ~Logger();

    Logger& Log(const char* text);
    Logger& LogPtr(const void* ptr);
    Logger& LogInt(int v);
    Logger& LogHex(unsigned long long v);
    Logger& LogFloat(float v);
};
} // namespace SenPatcher
