#include <string_view>

#include "modload/loaded_mods.h"
#include "tx/file_fixes.h"
#include "tx/file_fixes_sw.h"
#include "util/file.h"
#include "util/logger.h"

int main(int argc, char** argv) {
    HyoutaUtils::Logger logger(
        HyoutaUtils::IO::File(std::string_view("txtest.log"), HyoutaUtils::IO::OpenMode::Write));
    SenLib::ModLoad::LoadedP3AData vanillaP3As;
    SenLib::TX::CreateAssetPatchIfNeeded(
        logger, "f:\\SteamLibrary\\steamapps\\common\\Tokyo Xanadu eX+", vanillaP3As);
    SenLib::TX::CreateSwitchScriptArchive(logger,
                                          "f:\\_tx_switch\\data",
                                          "f:\\SteamLibrary\\steamapps\\common\\Tokyo Xanadu eX+",
                                          vanillaP3As);
    return 0;
}
