#include "sen1/file_fixes.h"

#include "modload/loaded_mods.h"

#include "util/logger.h"

int main(int argc, char** argv) {
    HyoutaUtils::Logger logger;
    SenLib::ModLoad::LoadedP3AData vanillaP3As;
    SenLib::Sen1::CreateAssetPatchIfNeeded(
        logger, "f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel", vanillaP3As);
    return 0;
}
