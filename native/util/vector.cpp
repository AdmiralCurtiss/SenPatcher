#include "vector.h"

#include <cstring>
#include <vector>

namespace HyoutaUtils::Vector {
bool ShiftData(std::span<char> data, size_t sourceLocation, size_t targetLocation, size_t length) {
    // bounds checks
    if (sourceLocation >= data.size() || length > data.size()
        || sourceLocation > (data.size() - length) || targetLocation > data.size()) {
        return false;
    }

    if (length == 0 || sourceLocation == targetLocation) {
        return true;
    }

    size_t lengthToShift;
    if (sourceLocation < targetLocation) {
        if (targetLocation < sourceLocation + length) {
            return false;
        }

        lengthToShift = targetLocation - (sourceLocation + length);
    } else {
        lengthToShift = sourceLocation - targetLocation;
    }

    std::vector<char> tmp;
    tmp.resize(length);
    std::memcpy(tmp.data(), &data[sourceLocation], length);
    if (sourceLocation < targetLocation) {
        std::memmove(&data[sourceLocation], &data[sourceLocation + length], lengthToShift);
        std::memcpy(&data[targetLocation - length], tmp.data(), length);
    } else {
        std::memmove(
            &data[(sourceLocation + length) - lengthToShift], &data[targetLocation], lengthToShift);
        std::memcpy(&data[targetLocation], tmp.data(), length);
    }
    return true;
}
} // namespace HyoutaUtils::Vector
