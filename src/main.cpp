#include "pl/Gloss.h"
#include <dobby.h>
#include "pl/Signature.h"
#include "util/Logger.hpp"
//#include "pl/internal/macro.h"
#include "util/InteractionResult.hpp"
#include "world/item/ItemStack.hpp"
#include "world/item/Item.hpp"
#include "world/item/registry/ItemRegistry.hpp"
#include "world/gamemode/GameMode.hpp"
#include "world/item/registry/ItemRegistryRef.hpp"
#include "world/actor/player/Player.hpp"
#include "world/actor/player/PlayerInventory.hpp"

#include <chrono>
#include <thread>

Logger logger("Offhand");

bool Hook(
    const char* sig,
    void* hook,
    void** orig
) {
    uintptr_t addr = pl::signature::pl_resolve_signature(sig, "libminecraftpe.so");
    if (!addr) {
        logger.error("signature not found");
        return false;
    }

    int ret = DobbyHook((void*)addr, hook, orig);
    if (ret != 0) {
        logger.error("DobbyHook failed: {}", ret);
        return false;
    }

    logger.info("Hook success");
    return true;
}

using registerItemsFn = void(*)(
        void*,
        void*,
        ItemRegistryRef,
        void*,
        void*
);

using useItemOnFn = InteractionResult* (*)(
        void*,
        ItemStack&,
        void*,
        void*,
        void*,
        void*,
        bool
);

using buildBlockFn = bool (*)(
        GameMode*,
        void*,
        void*,
        bool
);

static registerItemsFn registerItems_orig = nullptr;
static useItemOnFn useItemOn_orig = nullptr;
static buildBlockFn buildBlock_orig = nullptr;

void Hook_buildBlock() {
    uintptr_t addr = pl::signature::pl_resolve_signature(
        "FF C3 04 D1 FD 7B 0D A9 FC 73 00 F9 FA 67 0F A9 F8 5F 10 A9 F6 57 11 A9 F4 4F 12 A9 FD 43 03 91 59 D0 3B D5 F6 03 03 2A",
        "libminecraftpe.so"
    );
    if (!addr) {
        logger.error("Signature not found");
        return;
    }
    logger.info("Signature found!");
    int ret = DobbyHook(
        (void*)addr,
        (void*)buildBlock_hook,
        (void**)&buildBlock_orig
    );
    if (ret == 0) {
        logger.info("DobbyHook success buildBlock");
    } else {
        logger.error("DobbyHook failed buildBlock: {}", ret);
    }
}

void RegisterHooks() {
    Hook(
        "FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 FF 07 40 D1 FF 03 01 D1 48 D0 3B D5",
        (void*)registerItems_hook,
        (void**)&registerItems_orig
    );
    
    Hook(
        "EC 13 40 F9 E8 03 16 AA",
        (void*)useItemOn_hook,
        (void**)&useItemOn_orig
    );
    
    Hook(
        "FF C3 04 D1 FD 7B 0D A9 FC 73 00 F9 FA 67 0F A9 F8 5F 10 A9 F6 57 11 A9 F4 4F 12 A9 FD 43 03 91 59 D0 3B D5 F6 03 03 2A",
        (void*)buildBlock_hook,
        (void**)&buildBlock_orig
    );
    
    Hook(
        "FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 FF 03 1A D1 55 D0 3B D5",
        (void*)registerItems_hook,
        (void**)&registerItems_orig
    );
}

__attribute__((constructor))
void Init() {
    logger.info("Init()");
    GlossInit(true);
    Hook_registerItems();
    Hook_useItemOn();
}