#include "sen2/file_fixes.h"

#include <filesystem>

#include "logger.h"

int main(int argc, char** argv) {
    SenPatcher::Logger logger;
    SenLib::Sen2::CreateAssetPatchIfNeeded(
        logger, L"f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel II\\");
    return 0;
}
