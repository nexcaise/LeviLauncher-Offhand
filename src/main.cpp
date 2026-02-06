#include <pl/Gloss.h>
//#include <dobby.h>
//#include "pl/Signature.h"
#include "util/Logger.hpp"
//#include "pl/internal/macro.h"
/*
#include "util/InteractionResult.hpp"
#include "world/item/ItemStack.hpp"
#include "world/item/Item.hpp"
#include "world/item/registry/ItemRegistry.hpp"
#include "world/gamemode/GameMode.hpp"
#include "world/item/registry/ItemRegistryRef.hpp"
#include "world/actor/player/Player.hpp"
#include "world/actor/player/PlayerInventory.hpp"*/

#include "features/Hooks.hpp"

//#include <chrono>
//#include <thread>

Logger logger("Offhand");

__attribute__((constructor))
void Init() {
    logger.info("Init()");
    GlossInit(true);
    RegisterHooks();
}