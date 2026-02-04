#include "pl/Gloss.h"
#include <dobby.h>
#include "pl/Signature.h"
#include "util/Logger.hpp"
//#include "pl/internal/macro.h"
#include "util/InteractionResult.hpp"
#include "world/item/ItemStack.hpp"
#include "world/item/Item.hpp"
#include "world/item/registry/ItemRegistry.hpp"
#include "world/item/registry/ItemRegistryRef.hpp"

#include <chrono>
#include <thread>

Logger logger("Offhand");

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

static registerItemsFn registerItems_orig = nullptr;
static useItemOnFn useItemOn_orig = nullptr;

void registerItems_hook(
        void* self,
        void* ctx,
        ItemRegistryRef itemRegistry,
        void* baseGameVersion,
        void* experiments
) {
    if (registerItems_orig) {
        logger.info("registerItems_orig");
        registerItems_orig(self, ctx, itemRegistry, baseGameVersion, experiments);
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    auto sp = itemRegistry._lockRegistry();
    if (!sp) {
        logger.info("ItemRegistry Expired!");
        return;
    }
    
    for (auto& pair : sp.get()->mIdToItemMap)
    {
        pair.second.get()->setAllowOffhand(true);
    }
    
    logger.info("Success registry item!");
}

InteractionResult* useItemOn_hook(
        void* self,
        ItemStack& stack,
        void* at,
        void* face,
        void* hit,
        void* tb,
        bool iFE
) {
    Item* item = stack.getItem();

    if (!item)
    {
        logger.info("!item");
        return useItemOn_orig(self, stack, at, face, hit, tb, iFE);;
    }
    
    logger.info("Item namespace: {}", item->mNamespace);

    return useItemOn_orig(self, stack, at, face, hit, tb, iFE);
}

void Hook_useItemOn() {
    uintptr_t addr = pl::signature::pl_resolve_signature(
        "EC 13 40 F9 E8 03 16 AA",
        "libminecraftpe.so"
    );
    if (!addr) {
        logger.error("Signature not found");
        return;
    }
    logger.info("Signature found!");
    int ret = DobbyHook(
        (void*)addr,
        (void*)useItemOn_hook,
        (void**)&useItemOn_orig
    );
    if (ret == 0) {
        logger.info("DobbyHook success useItemOn");
    } else {
        logger.error("DobbyHook failed useItemOn: {}", ret);
    }
}

void Hook_registerItems() {
    uintptr_t addr = pl::signature::pl_resolve_signature(
        "FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 FF 07 40 D1 FF 03 01 D1 48 D0 3B D5",
        "libminecraftpe.so"
    );
    if (!addr) {
        logger.error("Signature not found");
        return;
    }
    logger.info("Signature found!");
    int ret = DobbyHook(
        (void*)addr,
        (void*)registerItems_hook,
        (void**)&registerItems_orig
    );
    if (ret == 0) {
        logger.info("DobbyHook success registerItems");
    } else {
        logger.error("DobbyHook failed registerItems: {}", ret);
    }
}

__attribute__((constructor))
void Init() {
    logger.info("Init()");
    GlossInit(true);
    Hook_registerItems();
    Hook_useItemOn();
}