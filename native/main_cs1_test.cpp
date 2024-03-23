#include "sen1/file_fixes.h"

#include <filesystem>

#include "logger.h"

int main(int argc, char** argv) {
    SenPatcher::Logger logger;
    SenLib::Sen1::CreateAssetPatchIfNeeded(
        logger, L"f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel\\");
    return 0;
}
