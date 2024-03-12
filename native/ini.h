#pragma once

#include <memory>
#include <span>
#include <string_view>

#include "file.h"

namespace SenPatcher {
struct IniKeyValueView {
    std::string_view Section;
    std::string_view Key;
    std::string_view Value;
};

struct IniFile {
    IniFile();
    IniFile(const IniFile& other) = delete;
    IniFile(IniFile&& other) = delete;
    IniFile& operator=(const IniFile& other) = delete;
    IniFile& operator=(IniFile&& other) = delete;
    ~IniFile();

    bool ParseFile(SenPatcher::IO::File& file);

    std::span<const IniKeyValueView> GetValues() const;
    const IniKeyValueView* FindValue(std::string_view section, std::string_view key) const;

private:
    // stores the ini file
    std::unique_ptr<char[]> DataBuffer = nullptr;
    size_t DataBufferLength = 0;

    // these are all references into the DataBuffer
    std::unique_ptr<IniKeyValueView[]> Values = nullptr;
    size_t ValueCount = 0;
};
} // namespace SenPatcher
