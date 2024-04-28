#include "sen2/file_fixes.h"

#include "util/logger.h"

int main(int argc, char** argv) {
    SenPatcher::Logger logger;
    SenLib::Sen2::CreateAssetPatchIfNeeded(
        logger, "f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel II");
    return 0;
}
