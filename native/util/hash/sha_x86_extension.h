#pragma once

#include <cstdint>

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#define HAS_SHA_X86_EXTENSION

#define SHA_X68_EXTENSION_BIG_ENDIAN_ORDER 0
#define SHA_X68_EXTENSION_LITTLE_ENDIAN_ORDER 1

void SHA1_HashMultipleBlocks_SHANI(uint32_t* state, const uint32_t* data, size_t length, int order);
void SHA256_HashMultipleBlocks_SHANI(uint32_t* state,
                                     const uint32_t* data,
                                     size_t length,
                                     int order);
#endif
