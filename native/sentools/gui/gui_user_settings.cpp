#include "gui_user_settings.h"

#include <string>
#include <string_view>

#include "util/file.h"
#include "util/ini.h"
#include "util/ini_writer.h"

namespace SenTools {
bool LoadUserSettingsFromIni(GuiUserSettings& settings, std::string_view path) {
    HyoutaUtils::IO::File file(path, HyoutaUtils::IO::OpenMode::Read);
    if (!file.IsOpen()) {
        return false;
    }
    HyoutaUtils::Ini::IniFile ini;
    if (!ini.ParseFile(file)) {
        return false;
    }
    return LoadUserSettingsFromIni(settings, ini);
}

bool LoadUserSettingsFromIni(GuiUserSettings& settings, const HyoutaUtils::Ini::IniFile& ini) {
    auto* sen1path = ini.FindValue("GamePaths", "Sen1Path");
    if (sen1path) {
        settings.Sen1Path = std::string(sen1path->Value);
    }
    auto* sen2path = ini.FindValue("GamePaths", "Sen2Path");
    if (sen2path) {
        settings.Sen2Path = std::string(sen2path->Value);
    }
    auto* sen3path = ini.FindValue("GamePaths", "Sen3Path");
    if (sen3path) {
        settings.Sen3Path = std::string(sen3path->Value);
    }
    auto* sen4path = ini.FindValue("GamePaths", "Sen4Path");
    if (sen4path) {
        settings.Sen4Path = std::string(sen4path->Value);
    }
    auto* sen5path = ini.FindValue("GamePaths", "Sen5Path");
    if (sen5path) {
        settings.Sen5Path = std::string(sen5path->Value);
    }
    auto* txpath = ini.FindValue("GamePaths", "TXPath");
    if (txpath) {
        settings.TXPath = std::string(txpath->Value);
    }
    return true;
}

bool WriteUserSettingsToIni(const GuiUserSettings& settings, std::string_view path) {
    // read existing ini first to keep values from eg. later senpatcher versions
    HyoutaUtils::Ini::IniWriter writer;
    {
        HyoutaUtils::Ini::IniFile ini;
        HyoutaUtils::IO::File file(path, HyoutaUtils::IO::OpenMode::Read);
        if (file.IsOpen() && ini.ParseFile(file)) {
            writer.AddExistingIni(ini);
        }
    }
    if (!WriteUserSettingsToIni(settings, writer)) {
        return false;
    }
    const std::string text = writer.GenerateIniText();
    return HyoutaUtils::IO::WriteFileAtomic(path, text.data(), text.size());
}

bool WriteUserSettingsToIni(const GuiUserSettings& settings, HyoutaUtils::Ini::IniWriter& ini) {
    ini.SetString("GamePaths", "Sen1Path", settings.Sen1Path);
    ini.SetString("GamePaths", "Sen2Path", settings.Sen2Path);
    ini.SetString("GamePaths", "Sen3Path", settings.Sen3Path);
    ini.SetString("GamePaths", "Sen4Path", settings.Sen4Path);
    ini.SetString("GamePaths", "Sen5Path", settings.Sen5Path);
    ini.SetString("GamePaths", "TXPath", settings.TXPath);
    return true;
}
} // namespace SenTools
