#include "sen4/file_fixes.h"

#include "logger.h"

int main(int argc, char** argv) {
    SenPatcher::Logger logger;
    SenLib::Sen4::CreateAssetPatchIfNeeded(
        logger,
        "f:\\SteamLibrary\\steamapps\\common\\The Legend of Heroes Trails of Cold Steel IV");
    return 0;
}
