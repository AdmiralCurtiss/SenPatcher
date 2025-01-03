#pragma once

#include <memory>
#include <span>
#include <string_view>

#include "util/file.h"

namespace HyoutaUtils::Ini {
// TODO: Add facility to unescape Value if it's a string with escape sequences.

// If Key/Value is empty then this node just represents the Comment made for a Section.
// If Section is empty then this node represents the Comment at the end of the file.
struct IniKeyValueView {
    std::string_view Section;
    std::string_view Key;
    std::string_view Value;
    std::string_view Comment;
};

struct IniFile {
    IniFile();
    IniFile(const IniFile& other) = delete;
    IniFile(IniFile&& other) = delete;
    IniFile& operator=(const IniFile& other) = delete;
    IniFile& operator=(IniFile&& other) = delete;
    ~IniFile();

    // IniFile takes memory ownership of data.
    // Returns false on IO error or parsing error (malformed ini).
    bool ParseFile(HyoutaUtils::IO::File& file);

    // IniFile takes memory ownership of data.
    // Returns false on parsing error (malformed ini).
    bool ParseMemory(std::unique_ptr<char[]> buffer, size_t bufferLength);

    // IniFile does *not* take memory ownership of data, but instead just points at it.
    // Ensure that the memory is deleted *after* IniFile is destructed.
    // Returns false on parsing error (malformed ini).
    bool ParseExternalMemory(const char* buffer, size_t bufferLength);

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
} // namespace HyoutaUtils::Ini
