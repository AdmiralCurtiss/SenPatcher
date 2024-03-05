#include "sha1.h"

#include <cstdint>

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

/// \brief Performs a left rotate
/// \tparam R the number of bit positions to rotate the value
/// \tparam T the word type
/// \param x the value to rotate
/// \details This is a portable C/C++ implementation. The value x to be rotated can be 8 to 64-bits wide.
/// \details R must be in the range <tt>[0, sizeof(T)*8 - 1]</tt> to avoid undefined behavior.
///  Use rotlMod if the rotate amount R is outside the range.
/// \details Use rotlConstant when the rotate amount is constant. The template function was added
///  because Clang did not propagate the constant when passed as a function parameter. Clang's
///  need for a constexpr meant rotlFixed failed to compile on occasion.
/// \note rotlConstant attempts to enlist a <tt>rotate IMM</tt> instruction because its often faster
///  than a <tt>rotate REG</tt>. Immediate rotates can be up to three times faster than their register
///  counterparts.
/// \sa rotlConstant, rotrConstant, rotlFixed, rotrFixed, rotlVariable, rotrVariable
/// \since Crypto++ 6.0
template <unsigned int R, class T> inline T rotlConstant(T x)
{
	// Portable rotate that reduces to single instruction...
	// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=57157,
	// http://software.intel.com/en-us/forums/topic/580884
	// and http://llvm.org/bugs/show_bug.cgi?id=24226
	CRYPTOPP_CONSTANT(THIS_SIZE = sizeof(T)*8);
	CRYPTOPP_CONSTANT(MASK = THIS_SIZE-1);
	static_assert(static_cast<int>(R) < THIS_SIZE);
	return T((x<<R)|(x>>(-R&MASK)));
}

////////////////////////////////
// start of Steve Reid's code //
////////////////////////////////

#define blk0(i) (W[i] = data[i])
#define blk1(i) (W[i&15] = rotlConstant<1>(W[(i+13)&15]^W[(i+8)&15]^W[(i+2)&15]^W[i&15]))

#define f1(x,y,z) (z^(x&(y^z)))
#define f2(x,y,z) (x^y^z)
#define f3(x,y,z) ((x&y)|(z&(x|y)))
#define f4(x,y,z) (x^y^z)

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=f1(w,x,y)+blk0(i)+0x5A827999+rotlConstant<5>(v);w=rotlConstant<30>(w);
#define R1(v,w,x,y,z,i) z+=f1(w,x,y)+blk1(i)+0x5A827999+rotlConstant<5>(v);w=rotlConstant<30>(w);
#define R2(v,w,x,y,z,i) z+=f2(w,x,y)+blk1(i)+0x6ED9EBA1+rotlConstant<5>(v);w=rotlConstant<30>(w);
#define R3(v,w,x,y,z,i) z+=f3(w,x,y)+blk1(i)+0x8F1BBCDC+rotlConstant<5>(v);w=rotlConstant<30>(w);
#define R4(v,w,x,y,z,i) z+=f4(w,x,y)+blk1(i)+0xCA62C1D6+rotlConstant<5>(v);w=rotlConstant<30>(w);

void SHA1_HashBlock_CXX(word32 *state, const word32 *data)
{
    word32 W[16];
    /* Copy context->state[] to working vars */
    word32 a = state[0];
    word32 b = state[1];
    word32 c = state[2];
    word32 d = state[3];
    word32 e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

#undef blk0
#undef blk1
#undef f1
#undef f2
#undef f3
#undef f4
#undef R1
#undef R2
#undef R3
#undef R4

}

//////////////////////////////
// end of Steve Reid's code //
//////////////////////////////

static void SHA1_InitState(word32 *state)
{
    state[0] = 0x67452301;
    state[1] = 0xEFCDAB89;
    state[2] = 0x98BADCFE;
    state[3] = 0x10325476;
    state[4] = 0xC3D2E1F0;
}

static void SHA1_All(word32* digest, const char* data, size_t length)
{
    word32 buffer[16];

    SHA1_InitState(digest);
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

        SHA1_HashBlock_CXX(digest, buffer);

        rest -= 64;
        input += 64;
    }

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
        SHA1_HashBlock_CXX(digest, buffer);
        for (i = 0; i < 14; ++i) {
            buffer[i] = 0;
        }
    }

    const word32 lowbits = (word32)((length*8) & 0xffffffff);
    const word32 highbits = (word32)((length*8) >> 32);
    buffer[14] = highbits;
    buffer[15] = lowbits;

    SHA1_HashBlock_CXX(digest, buffer);
}

}

namespace SenPatcher {
SHA1 CalculateSHA1(void* data, size_t length) {
    word32 state[5];
    SHA1_All(state, ((const char*)data), length);

    SHA1 rv;
    for (size_t i = 0; i < 5; ++i) {
        rv.Hash[i * 4 + 0] = (char)(state[i] >> 24);
        rv.Hash[i * 4 + 1] = (char)(state[i] >> 16);
        rv.Hash[i * 4 + 2] = (char)(state[i] >> 8);
        rv.Hash[i * 4 + 3] = (char)(state[i]);
    }
    return rv;
}
} // namespace SenPatcher
