#include "sen1/file_fixes.h"

#include "modload/loaded_mods.h"
#include "modload/loaded_pka.h"

#include "util/logger.h"

int main(int argc, char** argv) {
    HyoutaUtils::Logger logger;
    SenLib::ModLoad::LoadedP3AData vanillaP3As;
    SenLib::ModLoad::LoadedPkaData vanillaPKAs;
    SenLib::Sen1::CreateAssetPatchIfNeeded(
        logger,
        "f:\\SteamLibrary\\steamapps\\common\\Trails of Cold Steel",
        vanillaP3As,
        vanillaPKAs,
        {});
    return 0;
}
