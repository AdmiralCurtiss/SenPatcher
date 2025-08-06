#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace HyoutaUtils::Ini {
struct IniFile;

namespace Writer {
struct Value {
    std::string Name;
    std::string Data;
    std::string Comment;
};

struct Section {
    std::string Name;
    std::vector<Value> Values;
    std::string Comment;

    void AppendValues(std::string& text) const;

    void Insert(std::string_view name,
                std::string_view data,
                std::string_view comment,
                bool overwriteValue,
                bool appendToComment);

    void AddComment(std::string_view comment);
    void SetComment(std::string_view comment);
};
} // namespace Writer

struct IniWriter {
    std::vector<Writer::Section> Sections;
    std::string EndOfFileComment;

    IniWriter();
    IniWriter(const IniWriter& other);
    IniWriter(IniWriter&& other);
    IniWriter& operator=(const IniWriter& other);
    IniWriter& operator=(IniWriter&& other);
    ~IniWriter();

    Writer::Section& MakeOrGetSection(std::string_view name);
    Writer::Section* GetSection(std::string_view name);

    void SetBool(std::string_view sectionName, std::string_view keyName, bool boolValue);
    void SetInt(std::string_view sectionName, std::string_view keyName, int intValue);
    void SetInt64(std::string_view sectionName, std::string_view keyName, int64_t intValue);
    void SetUInt64(std::string_view sectionName, std::string_view keyName, uint64_t intValue);
    void SetDouble(std::string_view sectionName, std::string_view keyName, double doubleValue);
    void SetString(std::string_view sectionName,
                   std::string_view keyName,
                   std::string_view stringValue);

    void AddEndOfFileComment(std::string_view comment);
    void SetEndOfFileComment(std::string_view comment);

    void AddExistingIni(const IniFile& ini);

    std::string GenerateIniText() const;
};
} // namespace HyoutaUtils::Ini
