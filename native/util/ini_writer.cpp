#include "ini_writer.h"

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "ini.h"

namespace {
static void AddCommentLine(std::string& existingComment, std::string_view newComment) {
    if (!newComment.empty()) {
        if (!existingComment.empty()) {
            existingComment.push_back('\n');
        }
        existingComment.append(newComment);
    }
}

static void AppendComment(std::string& text, std::string_view comment) {
    std::string_view rest = comment;
    while (!rest.empty()) {
        const size_t nextLineSeparator = rest.find_first_of("\r\n");
        std::string_view line = rest.substr(0, nextLineSeparator);
        rest = nextLineSeparator != std::string_view::npos ? rest.substr(nextLineSeparator + 1)
                                                           : std::string_view();
        if (!line.empty()) {
            text.push_back(';');
            text.append(line);
            text.push_back('\n');
        }
    }
}
} // namespace

namespace HyoutaUtils::Ini {
namespace Writer {
void Section::AppendValues(std::string& text) const {
    for (const Value& v : Values) {
        AppendComment(text, v.Comment);
        text.append(v.Name);
        text.push_back('=');
        text.append(v.Data);
        text.push_back('\n');
        text.push_back('\n');
    }
}

void Section::Insert(std::string_view name,
                     std::string_view data,
                     std::string_view comment,
                     bool overwriteValue,
                     bool appendToComment) {
    for (Value& v : Values) {
        if (name == v.Name) {
            if (overwriteValue) {
                v.Data = std::string(data);
                if (appendToComment) {
                    AddCommentLine(v.Comment, comment);
                } else {
                    v.Comment = std::string(comment);
                }
            }
            return;
        }
    }
    Values.emplace_back(Value{
        .Name = std::string(name), .Data = std::string(data), .Comment = std::string(comment)});
}

void Section::AddComment(std::string_view comment) {
    AddCommentLine(Comment, comment);
}

void Section::SetComment(std::string_view comment) {
    Comment = std::string(comment);
}
} // namespace Writer

IniWriter::IniWriter() = default;
IniWriter::IniWriter(const IniWriter& other) = default;
IniWriter::IniWriter(IniWriter&& other) = default;
IniWriter& IniWriter::operator=(const IniWriter& other) = default;
IniWriter& IniWriter::operator=(IniWriter&& other) = default;
IniWriter::~IniWriter() = default;

Writer::Section& IniWriter::MakeOrGetSection(std::string_view name) {
    Writer::Section* sec = GetSection(name);
    if (sec == nullptr) {
        return Sections.emplace_back(Writer::Section{.Name = std::string(name)});
    }
    return *sec;
}

Writer::Section* IniWriter::GetSection(std::string_view name) {
    for (Writer::Section& s : Sections) {
        if (name == s.Name) {
            return &s;
        }
    }
    return nullptr;
}

void IniWriter::SetBool(std::string_view sectionName, std::string_view keyName, bool boolValue) {
    SetString(sectionName, keyName, boolValue ? "true" : "false");
}

void IniWriter::SetInt(std::string_view sectionName, std::string_view keyName, int intValue) {
    SetString(sectionName, keyName, std::to_string(intValue));
}

void IniWriter::SetInt64(std::string_view sectionName, std::string_view keyName, int64_t intValue) {
    SetString(sectionName, keyName, std::to_string(intValue));
}

void IniWriter::SetUInt64(std::string_view sectionName,
                          std::string_view keyName,
                          uint64_t intValue) {
    SetString(sectionName, keyName, std::to_string(intValue));
}

void IniWriter::SetDouble(std::string_view sectionName,
                          std::string_view keyName,
                          double doubleValue) {
    SetString(sectionName, keyName, std::to_string(doubleValue));
}

void IniWriter::SetString(std::string_view sectionName,
                          std::string_view keyName,
                          std::string_view stringValue) {
    MakeOrGetSection(sectionName).Insert(keyName, stringValue, std::string_view(), true, true);
}

void IniWriter::AddEndOfFileComment(std::string_view comment) {
    AddCommentLine(EndOfFileComment, comment);
}

void IniWriter::SetEndOfFileComment(std::string_view comment) {
    EndOfFileComment = std::string(comment);
}

void IniWriter::AddExistingIni(const IniFile& ini) {
    const auto strip_comment = [](std::string_view comment) -> std::string {
        std::string stripped;
        std::string_view rest = comment;
        while (!rest.empty()) {
            const size_t nextLineSeparator = rest.find_first_of("\r\n");
            std::string_view line = rest.substr(0, nextLineSeparator);
            rest = nextLineSeparator != std::string_view::npos ? rest.substr(nextLineSeparator + 1)
                                                               : std::string_view();
            if (line.starts_with(';')) {
                line = line.substr(1);
            }
            if (!line.empty()) {
                if (!stripped.empty()) {
                    stripped.push_back('\n');
                }
                stripped.append(line);
            }
        }
        return stripped;
    };

    for (const auto& entry : ini.GetValues()) {
        std::string comment = strip_comment(entry.Comment);
        if (entry.Key.empty()) {
            if (entry.Section.empty()) {
                this->SetEndOfFileComment(comment);
            } else {
                this->MakeOrGetSection(entry.Section).SetComment(comment);
            }
        } else {
            this->MakeOrGetSection(entry.Section)
                .Insert(entry.Key, entry.Value, comment, true, false);
        }
    }
}

std::string IniWriter::GenerateIniText() const {
    std::string text;

    for (const Writer::Section& section : Sections) {
        AppendComment(text, section.Comment);
        text.push_back('[');
        text.append(section.Name);
        text.push_back(']');
        text.push_back('\n');
        text.push_back('\n');
        section.AppendValues(text);
    }

    AppendComment(text, EndOfFileComment);

    return text;
}
} // namespace HyoutaUtils::Ini
