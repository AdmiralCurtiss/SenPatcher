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

    ParseMemory(std::move(buffer), *length);
    return true;
}

void IniFile::ParseExternalMemory(char* buffer, size_t bufferLength) {
    using HyoutaUtils::TextUtils::Trim;

    std::vector<IniKeyValueView> values;
    std::string_view remainingIni(buffer, bufferLength);
    std::string_view currentSection;

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
            continue;
        }

        // is this a section?
        if (line.front() == '[') {
            if (line.back() != ']') {
                // malformed section, skip to next line
                continue;
            }

            // remove brackets and whitespace
            line.remove_prefix(1);
            line.remove_suffix(1);
            line = Trim(line);

            // remaining string is new section name
            currentSection = line;
            continue;
        }

        // should be a key/value pair, find equals sign
        const size_t equalsSign = line.find_first_of('=');
        if (equalsSign == std::string_view::npos) {
            // couldn't find one, skip to next line
            continue;
        }

        std::string_view key = Trim(line.substr(0, equalsSign));
        std::string_view value = Trim(line.substr(equalsSign + 1));
        if (key.empty() || value.empty()) {
            // this is not valid either
            continue;
        }

        // we should technically unescape strings here (in-place is fine), but all our SenPatcher
        // settings are boolean or integer so let's skip that for now...
        values.emplace_back(IniKeyValueView{.Section = currentSection, .Key = key, .Value = value});
    }

    Values = std::make_unique_for_overwrite<IniKeyValueView[]>(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        Values[i] = std::move(values[i]);
    }
    ValueCount = values.size();
}

void IniFile::ParseMemory(std::unique_ptr<char[]> buffer, size_t bufferLength) {
    ParseExternalMemory(buffer.get(), bufferLength);
    DataBuffer = std::move(buffer);
    DataBufferLength = bufferLength;
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
