#include "tx/file_fixes.h"

#include "modload/loaded_mods.h"

#include "util/logger.h"

int main(int argc, char** argv) {
    HyoutaUtils::Logger logger;
    SenLib::ModLoad::LoadedP3AData vanillaP3As;
    SenLib::TX::CreateAssetPatchIfNeeded(
        logger, "f:\\SteamLibrary\\steamapps\\common\\Tokyo Xanadu eX+", vanillaP3As);
    return 0;
}
