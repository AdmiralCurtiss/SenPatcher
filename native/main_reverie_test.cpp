#include "sen5/file_fixes.h"

#include "logger.h"

int main(int argc, char** argv) {
    SenPatcher::Logger logger;
    SenLib::Sen5::CreateAssetPatchIfNeeded(
        logger, "f:\\SteamLibrary\\steamapps\\common\\The Legend of Heroes Trails into Reverie");
    return 0;
}
