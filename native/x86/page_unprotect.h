#pragma once

#include "logger.h"

namespace SenPatcher::x86 {
struct PageUnprotect {
    PageUnprotect(SenPatcher::Logger& logger, void* addr, size_t length);
    PageUnprotect(const PageUnprotect& other) = delete;
    PageUnprotect(PageUnprotect&& other) = delete;
    PageUnprotect& operator=(const PageUnprotect& other) = delete;
    PageUnprotect& operator=(PageUnprotect&& other) = delete;
    ~PageUnprotect();

private:
    SenPatcher::Logger& Log;
    void* Address;
    size_t Length;
    unsigned long Attributes;
};
} // namespace SenPatcher::x86
