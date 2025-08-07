#include "gui_user_settings.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "util/file.h"
#include "util/ini.h"
#include "util/ini_writer.h"
#include "util/system.h"
#include "util/text.h"

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
    auto* jpConfirm = ini.FindValue("GuiBehavior", "GamepadSwapConfirmCancel");
    if (jpConfirm) {
        settings.GamepadSwapConfirmCancel =
            HyoutaUtils::TextUtils::CaseInsensitiveEquals(jpConfirm->Value, "true");
    }
    auto* useCustomFileBrowser = ini.FindValue("GuiBehavior", "UseCustomFileBrowser");
    if (useCustomFileBrowser) {
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(useCustomFileBrowser->Value, "Always")) {
            settings.UseCustomFileBrowser = GuiUserSettings_UseCustomFileBrowser::Always;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(useCustomFileBrowser->Value,
                                                                 "Never")) {
            settings.UseCustomFileBrowser = GuiUserSettings_UseCustomFileBrowser::Never;
        } else {
            settings.UseCustomFileBrowser = GuiUserSettings_UseCustomFileBrowser::Auto;
        }
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
    ini.SetBool("GuiBehavior", "GamepadSwapConfirmCancel", settings.GamepadSwapConfirmCancel);
    ini.SetString(
        "GuiBehavior",
        "UseCustomFileBrowser",
        settings.UseCustomFileBrowser == GuiUserSettings_UseCustomFileBrowser::Always  ? "Always"
        : settings.UseCustomFileBrowser == GuiUserSettings_UseCustomFileBrowser::Never ? "Never"
                                                                                       : "Auto");
    return true;
}

void LoadUserSettingsFromCSharpUserConfig(GuiUserSettings& settings,
                                          std::string_view guiSettingsFolder) {
    // this is the old XML config the C# GUI writes. we'll try to parse something out of it, but if
    // we can't, it's no big deal...

    // the structure here is:
    // [guiSettingsFolder]\[folder with generated name]\1.0.0.0\user.config
    // different builds can use a different generated name, so just find all of them and load them
    // in the order from oldest to newest
    struct P {
        std::string Path;
        std::filesystem::file_time_type Timestamp;
    };
    std::vector<P> possibleUserConfigPaths;
    {
        auto folderPath = HyoutaUtils::IO::FilesystemPathFromUtf8(guiSettingsFolder);
        auto relativeUserConfigPath =
            HyoutaUtils::IO::FilesystemPathFromUtf8("1.0.0.0/user.config");
        std::error_code ec{};
        std::filesystem::directory_iterator iterator(folderPath, ec);
        if (ec) {
            return;
        }
        while (iterator != std::filesystem::directory_iterator()) {
            if (iterator->is_directory()) {
                auto userConfigPath = iterator->path() / relativeUserConfigPath;
                auto timestamp = std::filesystem::last_write_time(userConfigPath, ec);
                if (!ec) {
                    possibleUserConfigPaths.emplace_back(
                        P{.Path = HyoutaUtils::IO::FilesystemPathToUtf8(userConfigPath),
                          .Timestamp = timestamp});
                }
            }
            iterator.increment(ec);
            if (ec) {
                return;
            }
        }
    }
    std::sort(possibleUserConfigPaths.begin(),
              possibleUserConfigPaths.end(),
              [](const P& lhs, const P& rhs) { return lhs.Timestamp < rhs.Timestamp; });
    for (const auto& p : possibleUserConfigPaths) {
        HyoutaUtils::IO::File file(std::string_view(p.Path), HyoutaUtils::IO::OpenMode::Read);
        if (!file.IsOpen()) {
            continue;
        }
        const auto length = file.GetLength();
        if (!length || *length > (1024u * 1024u)) {
            continue;
        }
        auto buffer = std::make_unique_for_overwrite<char[]>(*length);
        if (file.Read(buffer.get(), *length) != *length) {
            continue;
        }
        std::string_view text(buffer.get(), *length);

        // sloppily parse this. i don't want to add an actual XML parser just for this, and as
        // mentioned it's not really important that we get this right, it's just for a slight
        // convenience when upgrading from an older SenPatcher version that still used the C# GUI
        static constexpr std::string_view settingsStartKey = "<SenPatcherGui.Properties.Settings>";
        static constexpr std::string_view settingsEndKey = "</SenPatcherGui.Properties.Settings>";
        static constexpr std::string_view settingStartKey = "<setting";
        static constexpr std::string_view settingEndKey = "</setting>";
        static constexpr std::string_view valueStartKey = "<value>";
        static constexpr std::string_view valueEndKey = "</value>";
        auto settingsPos = text.find(settingsStartKey);
        if (settingsPos != std::string_view::npos) {
            text = text.substr(settingsPos + settingsStartKey.size());
            auto settingsEndPos = text.find(settingsEndKey);
            if (settingsEndPos != std::string_view::npos) {
                text = text.substr(0, settingsEndPos);
                while (true) {
                    auto settingPos = text.find(settingStartKey);
                    if (settingsPos == std::string_view::npos) {
                        break;
                    }
                    std::string_view setting = text.substr(settingPos + settingStartKey.size());
                    auto settingEndPos = setting.find(settingEndKey);
                    if (settingEndPos == std::string_view::npos) {
                        break;
                    }
                    text = setting.substr(settingEndPos + settingEndKey.size());
                    setting = setting.substr(0, settingEndPos);

                    auto attribsEndPos = setting.find('>');
                    if (attribsEndPos == std::string_view::npos) {
                        continue;
                    }
                    std::string_view attribs = setting.substr(0, attribsEndPos);
                    std::string_view value = setting.substr(attribsEndPos + 1);
                    auto valueStartPos = value.find(valueStartKey);
                    if (valueStartPos == std::string_view::npos) {
                        continue;
                    }
                    value = value.substr(valueStartPos + valueStartKey.size());
                    auto valueEndPos = value.find(valueEndKey);
                    if (valueEndPos == std::string_view::npos) {
                        continue;
                    }
                    value = value.substr(0, valueEndPos);
                    const auto unescape_value = [&]() -> std::string {
                        std::string result;
                        for (size_t i = 0; i < value.size(); ++i) {
                            const char c = value[i];
                            if (c == '&') {
                                std::string_view tmp = value.substr(i);
                                if (tmp.starts_with("&amp;")) {
                                    result.push_back('&');
                                    i += 4;
                                } else if (tmp.starts_with("&quot;")) {
                                    result.push_back('"');
                                    i += 5;
                                } else if (tmp.starts_with("&apos;")) {
                                    result.push_back('\'');
                                    i += 5;
                                } else if (tmp.starts_with("&lt;")) {
                                    result.push_back('<');
                                    i += 3;
                                } else if (tmp.starts_with("&gt;")) {
                                    result.push_back('>');
                                    i += 3;
                                } else {
                                    result.push_back(c);
                                }
                            } else {
                                result.push_back(c);
                            }
                        }
                        return result;
                    };

                    if (attribs.find("\"Sen1Path\"") != std::string_view::npos) {
                        settings.Sen1Path = unescape_value();
                    } else if (attribs.find("\"Sen2Path\"") != std::string_view::npos) {
                        settings.Sen2Path = unescape_value();
                    } else if (attribs.find("\"Sen3Path\"") != std::string_view::npos) {
                        settings.Sen3Path = unescape_value();
                    } else if (attribs.find("\"Sen4Path\"") != std::string_view::npos) {
                        settings.Sen4Path = unescape_value();
                    } else if (attribs.find("\"Sen5Path\"") != std::string_view::npos) {
                        settings.Sen5Path = unescape_value();
                    } else if (attribs.find("\"TXPath\"") != std::string_view::npos) {
                        settings.TXPath = unescape_value();
                    }
                }
            }
        }
    }
}

static std::string FindExistingPath(std::string_view configuredPath,
                                    std::span<const std::string_view> otherGamesConfiguredPaths,
                                    std::span<const std::string_view> foldersToCheck,
                                    std::string_view filenameToCheck) {
    std::string filePath;

    // first check the configured path itself
    if (configuredPath != "") {
        filePath.assign(configuredPath);
        filePath.push_back('/');
        filePath.append(filenameToCheck);
        if (HyoutaUtils::IO::FileExists(std::string_view(filePath))
            == HyoutaUtils::IO::ExistsResult::DoesExist) {
            filePath.resize(filePath.size() - (filenameToCheck.size() + 1));
            return filePath;
        }
    }

    // then check if we can use a different configured path to find the target one
    for (std::string_view p : otherGamesConfiguredPaths) {
        if (p != "") {
            auto pp = HyoutaUtils::IO::SplitPath(p).Directory;
            if (pp != "") {
                for (std::string_view f : foldersToCheck) {
                    filePath.assign(pp);
                    filePath.push_back('/');
                    filePath.append(f);
                    filePath.push_back('/');
                    filePath.append(filenameToCheck);
                    if (HyoutaUtils::IO::FileExists(std::string_view(filePath))
                        == HyoutaUtils::IO::ExistsResult::DoesExist) {
                        filePath.resize(filePath.size() - (filenameToCheck.size() + 1));
                        return filePath;
                    }
                }
            }
        }
    }

#ifdef BUILD_FOR_WINDOWS
    // didn't find anything, check for plausible default paths
    for (const auto& drive : HyoutaUtils::IO::GetLogicalDrives()) {
        for (std::string_view f : foldersToCheck) {
            filePath.assign(drive);
            if (!(drive.ends_with('/') || drive.ends_with('\\'))) {
                filePath.push_back('/');
            }
            filePath.append("Program Files (x86)/Steam/steamapps/common/");
            filePath.append(f);
            filePath.push_back('/');
            filePath.append(filenameToCheck);
            if (HyoutaUtils::IO::FileExists(std::string_view(filePath))
                == HyoutaUtils::IO::ExistsResult::DoesExist) {
                filePath.resize(filePath.size() - (filenameToCheck.size() + 1));
                return filePath;
            }

            filePath.assign(drive);
            if (!(drive.ends_with('/') || drive.ends_with('\\'))) {
                filePath.push_back('/');
            }
            filePath.append("SteamLibrary/steamapps/common/");
            filePath.append(f);
            filePath.push_back('/');
            filePath.append(filenameToCheck);
            if (HyoutaUtils::IO::FileExists(std::string_view(filePath))
                == HyoutaUtils::IO::ExistsResult::DoesExist) {
                filePath.resize(filePath.size() - (filenameToCheck.size() + 1));
                return filePath;
            }

            filePath.assign(drive);
            if (!(drive.ends_with('/') || drive.ends_with('\\'))) {
                filePath.push_back('/');
            }
            filePath.append("Program Files (x86)/GOG Galaxy/Games/");
            filePath.append(f);
            filePath.push_back('/');
            filePath.append(filenameToCheck);
            if (HyoutaUtils::IO::FileExists(std::string_view(filePath))
                == HyoutaUtils::IO::ExistsResult::DoesExist) {
                filePath.resize(filePath.size() - (filenameToCheck.size() + 1));
                return filePath;
            }

            filePath.assign(drive);
            if (!(drive.ends_with('/') || drive.ends_with('\\'))) {
                filePath.push_back('/');
            }
            filePath.append("GOG Games/");
            filePath.append(f);
            filePath.push_back('/');
            filePath.append(filenameToCheck);
            if (HyoutaUtils::IO::FileExists(std::string_view(filePath))
                == HyoutaUtils::IO::ExistsResult::DoesExist) {
                filePath.resize(filePath.size() - (filenameToCheck.size() + 1));
                return filePath;
            }
        }
    }
#endif

    // in windows builds do the following block only under wine/proton, in linux builds do always
    std::string_view rootprefix;
#ifdef BUILD_FOR_WINDOWS
    rootprefix = "Z:";
    auto wineprefix = HyoutaUtils::Sys::GetEnvironmentVar("WINEPREFIX");
    if (wineprefix && !wineprefix->empty())
#endif
    {
        auto username = HyoutaUtils::Sys::GetEnvironmentVar("USER");
        if (!username) {
            username.emplace("deck"); // assume steam deck if we have nothing better
        }
        for (std::string_view f : foldersToCheck) {
            filePath.assign(rootprefix);
            filePath.append("/home/");
            filePath.append(*username);
            filePath.append("/.local/share/Steam/steamapps/common/");
            filePath.append(f);
            filePath.push_back('/');
            filePath.append(filenameToCheck);
            if (HyoutaUtils::IO::FileExists(std::string_view(filePath))
                == HyoutaUtils::IO::ExistsResult::DoesExist) {
                filePath.resize(filePath.size() - (filenameToCheck.size() + 1));
                return filePath;
            }

            filePath.assign(rootprefix);
            filePath.append("/home/");
            filePath.append(*username);
            filePath.append("/.steam/root/steamapps/common/");
            filePath.append(f);
            filePath.push_back('/');
            filePath.append(filenameToCheck);
            if (HyoutaUtils::IO::FileExists(std::string_view(filePath))
                == HyoutaUtils::IO::ExistsResult::DoesExist) {
                filePath.resize(filePath.size() - (filenameToCheck.size() + 1));
                return filePath;
            }
        }

        // check for mounted external drives, eg. SD card on steam deck
        static constexpr std::array<std::string_view, 3> mountroots{{
            "/run/media",
            "/media",
            "/mnt",
        }};
        for (std::string_view mountroot : mountroots) {
            std::error_code ec{};
            std::filesystem::directory_iterator iterator(
                HyoutaUtils::IO::FilesystemPathFromUtf8(std::string(rootprefix)
                                                        + std::string(mountroot)),
                ec);
            if (!ec) {
                while (iterator != std::filesystem::directory_iterator()) {
                    if (iterator->is_directory()) {
                        for (std::string_view f : foldersToCheck) {
                            filePath.assign(
                                HyoutaUtils::IO::FilesystemPathToUtf8(iterator->path()));
#ifdef BUILD_FOR_WINDOWS
                            if (!(filePath.ends_with('/') || filePath.ends_with('\\'))) {
#else
                            if (!filePath.ends_with('/')) {
#endif
                                filePath.push_back('/');
                            }
                            filePath.append("steamapps/common/");
                            filePath.append(f);
                            filePath.push_back('/');
                            filePath.append(filenameToCheck);
                            if (HyoutaUtils::IO::FileExists(std::string_view(filePath))
                                == HyoutaUtils::IO::ExistsResult::DoesExist) {
                                filePath.resize(filePath.size() - (filenameToCheck.size() + 1));
                                return filePath;
                            }
                        }
                    }
                    iterator.increment(ec);
                    if (ec) {
                        break;
                    }
                }
            }
        }
    }

    return "";
}

std::string GetDefaultPathCS1(const GuiUserSettings& settings) {
    std::array<std::string_view, 5> otherPaths{{
        settings.Sen2Path,
        settings.Sen3Path,
        settings.Sen4Path,
        settings.Sen5Path,
        settings.TXPath,
    }};
    static constexpr std::array<std::string_view, 2> possibleFolders{{
        "Trails of Cold Steel",
        "The Legend of Heroes - Trails of Cold Steel",
    }};
    return FindExistingPath(settings.Sen1Path, otherPaths, possibleFolders, "Sen1Launcher.exe");
}

std::string GetDefaultPathCS2(const GuiUserSettings& settings) {
    std::array<std::string_view, 5> otherPaths{{
        settings.Sen1Path,
        settings.Sen3Path,
        settings.Sen4Path,
        settings.Sen5Path,
        settings.TXPath,
    }};
    static constexpr std::array<std::string_view, 2> possibleFolders{{
        "Trails of Cold Steel II",
        "The Legend of Heroes Trails of Cold Steel II",
    }};
    return FindExistingPath(settings.Sen2Path, otherPaths, possibleFolders, "Sen2Launcher.exe");
}

std::string GetDefaultPathCS3(const GuiUserSettings& settings) {
    std::array<std::string_view, 5> otherPaths{{
        settings.Sen1Path,
        settings.Sen2Path,
        settings.Sen4Path,
        settings.Sen5Path,
        settings.TXPath,
    }};
    static constexpr std::array<std::string_view, 2> possibleFolders{{
        "The Legend of Heroes Trails of Cold Steel III",
        "ToCS3",
    }};
    return FindExistingPath(settings.Sen3Path, otherPaths, possibleFolders, "Sen3Launcher.exe");
}

std::string GetDefaultPathCS4(const GuiUserSettings& settings) {
    std::array<std::string_view, 5> otherPaths{{
        settings.Sen1Path,
        settings.Sen2Path,
        settings.Sen3Path,
        settings.Sen5Path,
        settings.TXPath,
    }};
    static constexpr std::array<std::string_view, 1> possibleFolders{{
        "The Legend of Heroes Trails of Cold Steel IV",
    }};
    return FindExistingPath(settings.Sen4Path, otherPaths, possibleFolders, "Sen4Launcher.exe");
}

std::string GetDefaultPathReverie(const GuiUserSettings& settings) {
    std::array<std::string_view, 5> otherPaths{{
        settings.Sen1Path,
        settings.Sen2Path,
        settings.Sen3Path,
        settings.Sen4Path,
        settings.TXPath,
    }};
    static constexpr std::array<std::string_view, 1> possibleFolders{{
        "The Legend of Heroes Trails into Reverie",
    }};
    return FindExistingPath(settings.Sen5Path, otherPaths, possibleFolders, "bin/Win64/hnk.exe");
}

std::string GetDefaultPathTX(const GuiUserSettings& settings) {
    std::array<std::string_view, 5> otherPaths{{
        settings.Sen1Path,
        settings.Sen2Path,
        settings.Sen3Path,
        settings.Sen4Path,
        settings.Sen5Path,
    }};
    static constexpr std::array<std::string_view, 1> possibleFolders{{
        "Tokyo Xanadu eX+",
    }};
    return FindExistingPath(settings.TXPath, otherPaths, possibleFolders, "TokyoXanadu.exe");
}

bool EvalUseCustomFileBrowser(const GuiUserSettings& settings) {
    // Don't use the native dialog if we're in a context where we want a controller-navigable UI.
    return settings.UseCustomFileBrowser == GuiUserSettings_UseCustomFileBrowser::Always ? true
           : settings.UseCustomFileBrowser == GuiUserSettings_UseCustomFileBrowser::Never
               ? false
               : (HyoutaUtils::Sys::GetEnvironmentVar("SteamTenfoot") == "1"
                  || HyoutaUtils::Sys::GetEnvironmentVar("SteamDeck") == "1");
}
} // namespace SenTools
