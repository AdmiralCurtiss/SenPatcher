#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "util/file.h"

#include "p3a/structs.h"

typedef struct ZSTD_DDict_s ZSTD_DDict;

namespace SenPatcher {
static constexpr uint32_t P3AFlag_HasUncompressedHashes = 1;
static constexpr uint32_t P3AFlagMask = P3AFlag_HasUncompressedHashes;

struct P3A {
    uint64_t FileCount = 0;
    std::unique_ptr<SenPatcher::P3AFileInfo[]> FileInfo{};
    ZSTD_DDict* Dict = nullptr;
    HyoutaUtils::IO::File FileHandle{};

    P3A();
    P3A(const P3A& other) = delete;
    P3A(P3A&& other);
    P3A& operator=(const P3A& other) = delete;
    P3A& operator=(P3A&& other);
    ~P3A();

    void Clear();
    bool Load(std::string_view path, uint32_t* out_p3aFlags);

private:
    bool Load(HyoutaUtils::IO::File& f, uint32_t* out_p3aFlags);
};

} // namespace SenPatcher
