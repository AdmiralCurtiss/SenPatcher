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

    // load both user ini and the current defaults and combine them
    {
        HyoutaUtils::Ini::IniFile defaultIni;
        if (!defaultIni.ParseExternalMemory(defaultIniString.data(), defaultIniString.size())) {
            return false;
        }
        writer.AddExistingIni(defaultIni);
    }

    // failing to parse the user ini is okay, in that case we just overwrite it
    {
        HyoutaUtils::Ini::IniFile userIni;
        HyoutaUtils::IO::File f(std::string_view(gameIniPath), HyoutaUtils::IO::OpenMode::Read);
        if (f.IsOpen()) {
            userIni.ParseFile(f);
        }
        writer.AddExistingIni(userIni);
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
        if (HyoutaUtils::IO::FileExists(std::string_view(gameDllPath))
            == HyoutaUtils::IO::ExistsResult::DoesExist) {
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
