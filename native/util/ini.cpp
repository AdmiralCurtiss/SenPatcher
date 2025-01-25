#include "ini.h"

#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "util/file.h"
#include "util/text.h"

namespace HyoutaUtils::Ini {
IniFile::IniFile() = default;
IniFile::~IniFile() = default;

bool IniFile::ParseFile(HyoutaUtils::IO::File& file) {
    if (!file.IsOpen()) {
        return false;
    }
    const auto length = file.GetLength();
    if (!length) {
        return false;
    }
    auto buffer = std::make_unique_for_overwrite<char[]>(*length);
    if (file.Read(buffer.get(), *length) != *length) {
        return false;
    }

    return ParseMemory(std::move(buffer), *length);
}

bool IniFile::ParseExternalMemory(const char* buffer, size_t bufferLength) {
    using HyoutaUtils::TextUtils::Trim;

    std::vector<IniKeyValueView> values;
    std::string_view remainingIni(buffer, bufferLength);
    std::string_view currentSection;
    std::string_view currentComment;

    // skip UTF8 BOM if it's there
    if (remainingIni.starts_with("\xef\xbb\xbf")) {
        remainingIni = remainingIni.substr(3);
    }

    while (!remainingIni.empty()) {
        const size_t nextLineSeparator = remainingIni.find_first_of("\r\n");
        std::string_view line = remainingIni.substr(0, nextLineSeparator);
        remainingIni = nextLineSeparator != std::string_view::npos
                           ? remainingIni.substr(nextLineSeparator + 1)
                           : std::string_view();

        // strip leading and trailing whitespace
        line = Trim(line);
        if (line.empty()) {
            continue;
        }

        // is this a comment? if yes skip to next line
        if (line.front() == ';') {
            if (currentComment.empty()) {
                currentComment = line;
            } else {
                // combine the content between the start of the comment and the current line end
                currentComment = std::string_view(currentComment.data(), line.data() + line.size());
            }
            continue;
        }

        // is this a section?
        if (line.front() == '[') {
            if (line.back() != ']') {
                // malformed section
                return false;
            }

            // remove brackets and whitespace
            line.remove_prefix(1);
            line.remove_suffix(1);
            line = Trim(line);

            // remaining string is new section name
            currentSection = line;

            if (!currentComment.empty()) {
                // create a value entry for the section comment
                values.emplace_back(
                    IniKeyValueView{.Section = currentSection, .Comment = currentComment});
                currentComment = std::string_view();
            }

            continue;
        }

        // should be a key/value pair, find equals sign
        const size_t equalsSign = line.find_first_of('=');
        if (equalsSign == std::string_view::npos) {
            // couldn't find one, malformed key/value pair
            return false;
        }

        std::string_view key = Trim(line.substr(0, equalsSign));
        if (key.empty()) {
            // this is not valid either
            return false;
        }

        std::string_view value = Trim(line.substr(equalsSign + 1));
        values.emplace_back(IniKeyValueView{
            .Section = currentSection, .Key = key, .Value = value, .Comment = currentComment});
        currentComment = std::string_view();
    }

    if (!currentComment.empty()) {
        // create a value entry for the remaining comment
        values.emplace_back(IniKeyValueView{.Comment = currentComment});
    }

    Values = std::make_unique_for_overwrite<IniKeyValueView[]>(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        Values[i] = std::move(values[i]);
    }
    ValueCount = values.size();
    return true;
}

bool IniFile::ParseMemory(std::unique_ptr<char[]> buffer, size_t bufferLength) {
    if (!ParseExternalMemory(buffer.get(), bufferLength)) {
        return false;
    }
    DataBuffer = std::move(buffer);
    DataBufferLength = bufferLength;
    return true;
}

std::span<const IniKeyValueView> IniFile::GetValues() const {
    return std::span<const IniKeyValueView>(Values.get(), ValueCount);
}

const IniKeyValueView* IniFile::FindValue(std::string_view section, std::string_view key) const {
    for (const auto& kvp : GetValues()) {
        if (kvp.Section == section && kvp.Key == key) {
            return &kvp;
        }
    }
    return nullptr;
}
} // namespace HyoutaUtils::Ini
