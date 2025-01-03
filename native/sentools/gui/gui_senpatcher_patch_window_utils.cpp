#include "gui_senpatcher_patch_window_utils.h"

#include <functional>
#include <string>
#include <string_view>

#include "util/ini.h"
#include "util/ini_writer.h"

namespace SenTools::GUI {
bool WriteGameSettingsIni(const std::function<void(HyoutaUtils::Ini::IniWriter& writer)>& callback,
                          const std::string& gameIniPath,
                          std::string_view defaultIniString) {
    HyoutaUtils::Ini::IniWriter writer;
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
    const auto add_to_writer = [&](HyoutaUtils::Ini::IniFile& ini) {
        for (const auto& entry : ini.GetValues()) {
            std::string comment = strip_comment(entry.Comment);
            if (entry.Key.empty()) {
                if (entry.Section.empty()) {
                    writer.SetEndOfFileComment(comment);
                } else {
                    writer.MakeOrGetSection(entry.Section).SetComment(comment);
                }
            } else {
                writer.MakeOrGetSection(entry.Section)
                    .Insert(entry.Key, entry.Value, comment, true, false);
            }
        }
    };

    // load both user ini and the current defaults and combine them
    {
        HyoutaUtils::Ini::IniFile defaultIni;
        if (!defaultIni.ParseExternalMemory(defaultIniString.data(), defaultIniString.size())) {
            return false;
        }
        add_to_writer(defaultIni);
    }

    // failing to parse the user ini is okay, in that case we just overwrite it
    {
        HyoutaUtils::Ini::IniFile userIni;
        HyoutaUtils::IO::File f(std::string_view(gameIniPath), HyoutaUtils::IO::OpenMode::Read);
        if (f.IsOpen()) {
            userIni.ParseFile(f);
        }
        add_to_writer(userIni);
    }

    if (callback) {
        callback(writer);
    }

    std::string text = writer.GenerateIniText();
    HyoutaUtils::IO::File outfile(std::string_view(gameIniPath), HyoutaUtils::IO::OpenMode::Write);
    if (!outfile.IsOpen()) {
        return false;
    }
    if (outfile.Write(text.data(), text.size()) != text.size()) {
        return false;
    }
    return true;
}

std::string
    PatchOrUnpatchGame(const std::function<void(HyoutaUtils::Ini::IniWriter& writer)>& callback,
                       const std::string& localDllPath,
                       const std::string& gameDllPath,
                       const std::string& gameIniPath,
                       std::string_view defaultIniString,
                       bool unpatch) {
    if (unpatch) {
        if (HyoutaUtils::IO::FileExists(std::string_view(gameDllPath))) {
            if (!HyoutaUtils::IO::DeleteFile(std::string_view(gameDllPath))) {
                return std::string("Deleting SenPatcher DLL failed.");
            }
            return std::string(
                "Removed SenPatcher. Files for installed mods have not been removed, but "
                "will no longer be loaded. To remove them, navigate to the game "
                "directory and delete the 'mods' folder.");
        } else {
            return std::string("Could not find anything to remove.");
        }
    } else {
        if (!WriteGameSettingsIni(callback, gameIniPath, defaultIniString)) {
            return std::string("Writing senpatcher_settings.ini failed.");
        }
        if (!HyoutaUtils::IO::CopyFile(localDllPath, gameDllPath)) {
            return std::string("Copying SenPatcher DLL failed.");
        }
        return std::string("SenPatcher DLL successfully copied to game directory.");
    }
}
} // namespace SenTools::GUI
