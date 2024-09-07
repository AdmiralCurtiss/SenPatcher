#include "page_unprotect.h"

#include <bit>
#include <cstdlib>

#include "util/logger.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace SenPatcher::x64 {
PageUnprotect::PageUnprotect(HyoutaUtils::Logger& logger, void* addr, size_t length) : Log(logger) {
    if (length == 0) {
        return;
    }

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    const unsigned long pageSize = sysinfo.dwPageSize;
    const uint64_t firstPageAddress =
        std::bit_cast<uint64_t>(addr) & (~(static_cast<uint64_t>(pageSize) - 1));
    const uint64_t lastPageAddress =
        (std::bit_cast<uint64_t>(addr) + (length - 1)) & (~(static_cast<uint64_t>(pageSize) - 1));

    FirstPageAddress = firstPageAddress;
    LastPageAddress = lastPageAddress;
    PageSize = pageSize;

    // unprotect first page
    Log.Log("Requested unprotect of ")
        .LogHex(length)
        .Log(" bytes at ")
        .LogPtr(addr)
        .Log(". Page size is ")
        .LogHex(pageSize)
        .Log(". Unprotecting ")
        .LogPtr(std::bit_cast<void*>(firstPageAddress));
    if (!VirtualProtect(
            std::bit_cast<void*>(firstPageAddress), pageSize, PAGE_READWRITE, &Attributes)) {
        Log.Log(" -> Failed.\n");
        std::exit(-1);
    }
    Log.Log(" -> Success, previous attributes were ").LogHex(Attributes).Log(".\n");

    // unprotect remaining pages, if any, and make sure their attributes match the first page
    // (otherwise we have a logic error somewhere)
    uint64_t currentPageAddress = firstPageAddress;
    while (true) {
        if (currentPageAddress == lastPageAddress) {
            break;
        }
        currentPageAddress += pageSize;

        unsigned long attr;
        Log.Log("Unprotecting next page ").LogPtr(std::bit_cast<void*>(currentPageAddress));
        if (!VirtualProtect(
                std::bit_cast<void*>(currentPageAddress), pageSize, PAGE_READWRITE, &attr)) {
            Log.Log(" -> Failed.\n");
            std::exit(-1);
        }
        if (Attributes != attr) {
            Log.Log(" -> Success, but attributes differ (were ").LogHex(attr).Log(").\n");
            std::exit(-1);
        }
        Log.Log(" -> Success.\n");
    }
}

PageUnprotect::~PageUnprotect() {
    uint64_t currentPageAddress = FirstPageAddress;
    const uint64_t lastPageAddress = LastPageAddress;
    const unsigned long pageSize = PageSize;
    const unsigned long reprotectAttributes = Attributes;

    while (true) {
        unsigned long attr;
        Log.Log("Reprotecting page ").LogPtr(std::bit_cast<void*>(currentPageAddress));
        if (!VirtualProtect(
                std::bit_cast<void*>(currentPageAddress), pageSize, reprotectAttributes, &attr)) {
            Log.Log(" -> Failed.\n");
            std::exit(-1);
        }
        Log.Log(" -> Success.\n");

        if (currentPageAddress == lastPageAddress) {
            break;
        }
        currentPageAddress += pageSize;
    }
}
} // namespace SenPatcher::x64
