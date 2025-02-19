#include "sha256.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "util/cpuid.h"

#include "sha_x86_extension.h"

// sha.cpp - modified by Wei Dai from Steve Reid's public domain sha1.c

//    Steve Reid implemented SHA-1. Wei Dai implemented SHA-2. Jeffrey
//    Walton implemented Intel SHA extensions based on Intel articles and code
//    by Sean Gulley. Jeffrey Walton implemented ARM SHA-1 and SHA-256 based
//    on ARM code and code from Johannes Schneiders, Skip Hovsmith and
//    Barry O'Rourke. Jeffrey Walton and Bill Schmidt implemented Power8
//    SHA-256 and SHA-512. All code is in the public domain.

//    In August 2017 JW reworked the internals to align all the
//    implementations. Formerly all hashes were software based, IterHashBase
//    handled endian conversions, and IterHashBase dispatched a single to
//    block SHA{N}::Transform. SHA{N}::Transform then performed the single
//    block hashing. It was repeated for multiple blocks.
//
//    The rework added SHA{N}::HashMultipleBlocks (class) and
//    SHA{N}_HashMultipleBlocks (free standing). There are also hardware
//    accelerated variations. Callers enter SHA{N}::HashMultipleBlocks (class)
//    and the function calls SHA{N}_HashMultipleBlocks (free standing) or
//    SHA{N}_HashBlock (free standing) as a fallback.
//
//    An added wrinkle is hardware is little endian, C++ is big endian, and
//    callers use big endian, so SHA{N}_HashMultipleBlock accepts a ByteOrder
//    for the incoming data arrangement. Hardware based SHA{N}_HashMultipleBlock
//    can often perform the endian swap much easier by setting an EPI mask.
//    Endian swap incurs no penalty on Intel SHA, and 4-instruction penalty on
//    ARM SHA. Under C++ the full software based swap penalty is incurred due
//    to use of ReverseBytes().
//
//    In May 2019 JW added Cryptogams ARMv7 and NEON implementations for SHA1,
//    SHA256 and SHA512. The Cryptogams code closed a performance gap on modern
//    32-bit ARM devices. Cryptogams is Andy Polyakov's project used to create
//    high speed crypto algorithms and share them with other developers. Andy's
//    code runs 30% to 50% faster than C/C++ code. The Cryptogams code can be
//    disabled in config_asm.h. An example of integrating Andy's code is at
//    https://wiki.openssl.org/index.php/Cryptogams_SHA.

namespace {
using word32 = uint32_t;

# define CRYPTOPP_CONSTANT(x) constexpr static int x

namespace {
static constexpr word32 SHA256_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/// \brief Performs a right rotate
/// \tparam R the number of bit positions to rotate the value
/// \tparam T the word type
/// \param x the value to rotate
/// \details This is a portable C/C++ implementation. The value x to be rotated can be 8 to 64-bits wide.
/// \details R must be in the range <tt>[0, sizeof(T)*8 - 1]</tt> to avoid undefined behavior.
///  Use rotrMod if the rotate amount R is outside the range.
/// \details Use rotrConstant when the rotate amount is constant. The template function was added
///  because Clang did not propagate the constant when passed as a function parameter. Clang's
///  need for a constexpr meant rotrFixed failed to compile on occasion.
/// \note rotrConstant attempts to enlist a <tt>rotate IMM</tt> instruction because its often faster
///  than a <tt>rotate REG</tt>. Immediate rotates can be up to three times faster than their register
///  counterparts.
/// \sa rotlConstant, rotrConstant, rotlFixed, rotrFixed, rotlVariable, rotrVariable
template <unsigned int R, class T> inline T rotrConstant(T x) noexcept
{
	CRYPTOPP_CONSTANT(THIS_SIZE = sizeof(T)*8);
	static_assert(static_cast<int>(R) < THIS_SIZE);
	return std::rotr(x, static_cast<int>(R));
}

#define a(i) T[(0-i)&7]
#define b(i) T[(1-i)&7]
#define c(i) T[(2-i)&7]
#define d(i) T[(3-i)&7]
#define e(i) T[(4-i)&7]
#define f(i) T[(5-i)&7]
#define g(i) T[(6-i)&7]
#define h(i) T[(7-i)&7]

#define blk0(i) (W[i] = data[i])
#define blk2(i) (W[i&15]+=s1(W[(i-2)&15])+W[(i-7)&15]+s0(W[(i-15)&15]))

#define Ch(x,y,z) (z^(x&(y^z)))
#define Maj(x,y,z) (y^((x^y)&(y^z)))

#define R(i) h(i)+=S1(e(i))+Ch(e(i),f(i),g(i))+SHA256_K[i+j]+(j?blk2(i):blk0(i));\
    d(i)+=h(i);h(i)+=S0(a(i))+Maj(a(i),b(i),c(i))

// for SHA256
#define s0(x) (rotrConstant<7>(x)^rotrConstant<18>(x)^(x>>3))
#define s1(x) (rotrConstant<17>(x)^rotrConstant<19>(x)^(x>>10))
#define S0(x) (rotrConstant<2>(x)^rotrConstant<13>(x)^rotrConstant<22>(x))
#define S1(x) (rotrConstant<6>(x)^rotrConstant<11>(x)^rotrConstant<25>(x))

void SHA256_HashBlock_CXX(word32 *state, const word32 *data) noexcept
{
    word32 W[16]={0}, T[8];
    /* Copy context->state[] to working vars */
    std::memcpy(T, state, sizeof(T));
    /* 64 operations, partially loop unrolled */
    for (unsigned int j=0; j<64; j+=16)
    {
        R( 0); R( 1); R( 2); R( 3);
        R( 4); R( 5); R( 6); R( 7);
        R( 8); R( 9); R(10); R(11);
        R(12); R(13); R(14); R(15);
    }
    /* Add the working vars back into context.state[] */
    state[0] += a(0);
    state[1] += b(0);
    state[2] += c(0);
    state[3] += d(0);
    state[4] += e(0);
    state[5] += f(0);
    state[6] += g(0);
    state[7] += h(0);
}

#undef Ch
#undef Maj
#undef s0
#undef s1
#undef S0
#undef S1
#undef blk0
#undef blk1
#undef blk2
#undef R

#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
#undef g
#undef h

}

static void SHA256_InitState(word32 *state) noexcept
{
    state[0] = 0x6a09e667;
    state[1] = 0xbb67ae85;
    state[2] = 0x3c6ef372;
    state[3] = 0xa54ff53a;
    state[4] = 0x510e527f;
    state[5] = 0x9b05688c;
    state[6] = 0x1f83d9ab;
    state[7] = 0x5be0cd19;
}

static size_t SHA256_Update(word32* digest, const char* data, size_t length) noexcept
{
#ifdef HAS_SHA_X86_EXTENSION
    static const bool hasShaCpuInstruction = HyoutaUtils::CpuId::SupportsSHA();
    const bool isDataAligned = ((std::bit_cast<size_t>(data) % 16) == 0);
    if (hasShaCpuInstruction && isDataAligned) {
        const size_t consume = length & ~static_cast<size_t>(0x3f);
        if (consume > 0) {
            SHA256_HashMultipleBlocks_SHANI(digest,
                                            reinterpret_cast<const uint32_t*>(data),
                                            consume,
                                            SHA_X68_EXTENSION_BIG_ENDIAN_ORDER);
        }
        return consume;
    }
#endif

    alignas(16) word32 buffer[16];
    size_t rest = length;
    const char* input = data;
    size_t i;
    while (rest >= 64) {
        for (i = 0; i < 16; ++i) {
            buffer[i] =   ((word32)((unsigned char)input[i * 4 + 3]))
                        | (((word32)((unsigned char)input[i * 4 + 2])) << 8)
                        | (((word32)((unsigned char)input[i * 4 + 1])) << 16)
                        | (((word32)((unsigned char)input[i * 4])) << 24);
        }

        SHA256_HashBlock_CXX(digest, buffer);

        rest -= 64;
        input += 64;
    }

    // number of bytes consumed
    return static_cast<size_t>(input - data);
}

static void SHA256_Rest(word32* digest, uint64_t totalByteLength, const char* data, size_t rest) noexcept {
    alignas(16) word32 buffer[16];
    const char* input = data;
    size_t i;

    // handle remaining bytes and '1' bit at the end of message
    for (i = 0; i < rest / 4; ++i) {
        buffer[i] =   ((word32)((unsigned char)input[i * 4 + 3]))
                    | (((word32)((unsigned char)input[i * 4 + 2])) << 8)
                    | (((word32)((unsigned char)input[i * 4 + 1])) << 16)
                    | (((word32)((unsigned char)input[i * 4])) << 24);
    }
    const size_t spillbytes = (rest % 4);
    if (spillbytes == 0) {
        buffer[i] = ((word32)(1 << 31));
    } else if (spillbytes == 1) {
        buffer[i] = (((word32)((unsigned char)input[i * 4])) << 24) | ((word32)(1 << 23));
    } else if (spillbytes == 2) {
        buffer[i] =   (((word32)((unsigned char)input[i * 4 + 1])) << 16)
                    | (((word32)((unsigned char)input[i * 4])) << 24) | ((word32)(1 << 15));
    } else {
        buffer[i] =   (((word32)((unsigned char)input[i * 4 + 2])) << 8)
                    | (((word32)((unsigned char)input[i * 4 + 1])) << 16)
                    | (((word32)((unsigned char)input[i * 4])) << 24) | ((word32)(1 << 7));
    }
    ++i;

    if (rest <= 55) {
        for (; i < 14; ++i) {
            buffer[i] = 0;
        }
    } else {
        // need an extra round because there's not enough space
        // for the message length in the current one
        for (; i < 16; ++i) {
            buffer[i] = 0;
        }
        SHA256_HashBlock_CXX(digest, buffer);
        for (i = 0; i < 14; ++i) {
            buffer[i] = 0;
        }
    }

    const word32 lowbits = (word32)((totalByteLength << 3) & 0xffffffff);
    const word32 highbits = (word32)((totalByteLength >> 29) & 0xffffffff);
    buffer[14] = highbits;
    buffer[15] = lowbits;

    SHA256_HashBlock_CXX(digest, buffer);
}

static void SHA256_All(word32* digest, const char* data, size_t length) noexcept {
    SHA256_InitState(digest);
    const size_t bytesConsumed = length > 0 ? SHA256_Update(digest, data, length) : 0;
    SHA256_Rest(digest, length, data + bytesConsumed, length - bytesConsumed);
}
}

namespace HyoutaUtils::Hash {
SHA256 CalculateSHA256(const void* data, size_t length) noexcept {
    alignas(16) word32 state[8];
    SHA256_All(state, ((const char*)data), length);

    SHA256 rv;
    for (size_t i = 0; i < 8; ++i) {
        rv.Hash[i * 4 + 0] = (char)(state[i] >> 24);
        rv.Hash[i * 4 + 1] = (char)(state[i] >> 16);
        rv.Hash[i * 4 + 2] = (char)(state[i] >> 8);
        rv.Hash[i * 4 + 3] = (char)(state[i]);
    }
    return rv;
}
} // namespace HyoutaUtils::Hash
