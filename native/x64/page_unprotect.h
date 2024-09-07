#pragma once

#include <cstdint>

#include "util/logger.h"

namespace SenPatcher::x64 {
struct PageUnprotect {
    PageUnprotect(HyoutaUtils::Logger& logger, void* addr, size_t length);
    PageUnprotect(const PageUnprotect& other) = delete;
    PageUnprotect(PageUnprotect&& other) = delete;
    PageUnprotect& operator=(const PageUnprotect& other) = delete;
    PageUnprotect& operator=(PageUnprotect&& other) = delete;
    ~PageUnprotect();

private:
    HyoutaUtils::Logger& Log;
    uint64_t FirstPageAddress = 0;
    uint64_t LastPageAddress = 0;
    unsigned long PageSize = 0;
    unsigned long Attributes = 0;
};
} // namespace SenPatcher::x64
