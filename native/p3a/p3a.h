#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>

#include "file.h"

#include "p3a/structs.h"

typedef struct ZSTD_DDict_s ZSTD_DDict;

namespace SenPatcher {
struct P3A {
    uint64_t FileCount = 0;
    std::unique_ptr<SenPatcher::P3AFileInfo[]> FileInfo{};
    ZSTD_DDict* Dict = nullptr;
    SenPatcher::IO::File FileHandle{};

    P3A();
    P3A(const P3A& other) = delete;
    P3A(P3A&& other);
    P3A& operator=(const P3A& other) = delete;
    P3A& operator=(P3A&& other);
    ~P3A();

    void Clear();
    bool Load(std::string_view path);
    bool Load(const std::filesystem::path& path);

private:
    bool Load(IO::File& f);
};

} // namespace SenPatcher
