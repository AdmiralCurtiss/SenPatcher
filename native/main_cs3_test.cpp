#include "sen3/file_fixes.h"

#include "logger.h"

int main(int argc, char** argv) {
    SenPatcher::Logger logger;
    SenLib::Sen3::CreateAssetPatchIfNeeded(
        logger,
        "f:\\SteamLibrary\\steamapps\\common\\The Legend of Heroes Trails of Cold Steel III");
    return 0;
}
