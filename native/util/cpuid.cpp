#include "cpuid.h"

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#include <intrin.h>
#endif

namespace HyoutaUtils::CpuId {
bool SupportsSHA() {
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
    int output[4];
    __cpuid(output, 0);
    int highestValidId = output[0];
    if (highestValidId >= 7) {
        __cpuid(output, 7);
        return (output[1] & (1 << 29)) != 0;
    }
#endif

    // TODO: Other compilers.
    return false;
}
} // namespace HyoutaUtils::CpuId
