#include "page_unprotect.h"

#include <bit>

#include "util/logger.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace SenPatcher::x64 {
PageUnprotect::PageUnprotect(HyoutaUtils::Logger& logger, void* addr, size_t length) : Log(logger) {
    // FIXME: check length/alignment, this might span multiple pages!
    Length = 0x1000;
    Address = std::bit_cast<void*>(std::bit_cast<uint64_t>(addr) & (~(Length - 1)));
    Log.Log("Unprotecting ").LogHex(Length).Log(" bytes at ").LogPtr(Address);
    if (VirtualProtect(Address, Length, PAGE_READWRITE, &Attributes)) {
        Log.Log(" -> Success, previous attributes were ").LogHex(Attributes).Log(".\n");
    } else {
        Log.Log(" -> Failed.\n");
    }
}

PageUnprotect::~PageUnprotect() {
    DWORD tmp;
    Log.Log("Reprotecting ").LogHex(Length).Log(" bytes at ").LogPtr(Address);
    Log.Log(" to attributes ").LogHex(Attributes);
    if (VirtualProtect(Address, Length, Attributes, &tmp)) {
        Log.Log(" -> Success.\n");
    } else {
        Log.Log(" -> Failed.\n");
    }
}
} // namespace SenPatcher::x64
