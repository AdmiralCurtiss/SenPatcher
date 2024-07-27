#include "sen2/file_fixes.h"

#include "modload/loaded_mods.h"

#include "util/logger.h"

int main(int argc, char** argv) {
    HyoutaUtils::Logger logger;
    SenLib::ModLoad::LoadedP3AData vanillaP3As;
    SenLib::Sen2::CreateAssetPatchIfNeeded(
        logger, "f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel II", vanillaP3As);
    return 0;
}
