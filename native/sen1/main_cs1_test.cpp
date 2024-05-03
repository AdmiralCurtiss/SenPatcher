#include "sen1/file_fixes.h"

#include "util/logger.h"

int main(int argc, char** argv) {
    HyoutaUtils::Logger logger;
    SenLib::Sen1::CreateAssetPatchIfNeeded(
        logger, "f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel");
    return 0;
}
