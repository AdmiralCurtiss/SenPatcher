#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace HyoutaUtils::Ini {
struct IniFile;
struct IniWriter;
} // namespace HyoutaUtils::Ini

namespace SenTools {
enum class GuiUserSettings_UseCustomFileBrowser : uint8_t {
    Auto,
    Always,
    Never, // this will still use the custom browser if we have no other option
};

struct GuiUserSettings {
    std::string Sen1Path;
    std::string Sen2Path;
    std::string Sen3Path;
    std::string Sen4Path;
    std::string Sen5Path;
    std::string TXPath;
    bool GamepadSwapConfirmCancel = false;
    GuiUserSettings_UseCustomFileBrowser UseCustomFileBrowser =
        GuiUserSettings_UseCustomFileBrowser::Auto;
};

bool LoadUserSettingsFromIni(GuiUserSettings& settings, std::string_view path);
bool LoadUserSettingsFromIni(GuiUserSettings& settings, const HyoutaUtils::Ini::IniFile& ini);
bool WriteUserSettingsToIni(const GuiUserSettings& settings, std::string_view path);
bool WriteUserSettingsToIni(const GuiUserSettings& settings, HyoutaUtils::Ini::IniWriter& ini);

void LoadUserSettingsFromCSharpUserConfig(GuiUserSettings& settings,
                                          std::string_view guiSettingsFolder);

std::string GetDefaultPathCS1(const GuiUserSettings& settings);
std::string GetDefaultPathCS2(const GuiUserSettings& settings);
std::string GetDefaultPathCS3(const GuiUserSettings& settings);
std::string GetDefaultPathCS4(const GuiUserSettings& settings);
std::string GetDefaultPathReverie(const GuiUserSettings& settings);
std::string GetDefaultPathTX(const GuiUserSettings& settings);
bool EvalUseCustomFileBrowser(const GuiUserSettings& settings);
} // namespace SenTools
