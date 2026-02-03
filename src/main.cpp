#include "pl/Gloss.h"
#include <dobby.h>
#include "pl/Signature.h"
#include "pl/Logger.h"
//#include "pl/internal/macro.h"
#include "util/InteractionResult.hpp"
#include "world/item/ItemStack.hpp"
#include "world/item/Item.hpp"

pl::log::Logger logger("Offhand");

using useItemOnFn = InteractionResult* (*)(
        void*,
        //InteractionResult*,
        ItemStack&,
        void*,
        void*,
        void*,
        void*,
        bool
);

static useItemOnFn useItemOn_orig = nullptr;

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
    InteractionResult* result = orig(self, stack, at, face, hit, tb, iFE);
    //bool realMayUse = stack.mItem == nullptr;//!stack.mValid || stack.mItem == nullptr || stack.isNull() ||/* stack.mCount == 0 || !isSimTick ||*/ !item/* || item->canUseOnSimTick()*/;

    logger.info("res: {}", result->mResult);
    if (!item)
    {
        logger.info("May not use item");
        //result->mResult = (int)InteractionResult::Result::SUCCESS | (int)InteractionResult::Result::SWING;//useItemOn_orig(self,stack,at,face,hit,tb,isFirstEvent);
        return result;
    }
    
    logger.info("allowOffhand: {}", item->mAllowOffhand);
    //item->setAllowOffhand(true);

    //result->mResult = (int)InteractionResult::Result::SUCCESS | (int)InteractionResult::Result::SWING;//useItemOn_orig(self,stack,at,face,hit,tb,isFirstEvent);
    return result;

}

__attribute__((constructor))
void Init() {
    GlossInit(true);
    uintptr_t useItemOn_addr = pl::signature::pl_resolve_signature(
        "EC 13 40 F9 E8 03 16 AA",
        "libminecraftpe.so"
    );
    if (!useItemOn_addr) {
        logger.error("Signature not found");
        return;
    }
    logger.info("Signature found!");
    int useItemOn_ret = DobbyHook(
        (void*)useItemOn_addr,
        (void*)useItemOn_hook,
        (void**)&useItemOn_orig
    );
    if (useItemOn_ret == 0) {
        logger.info("DobbyHook success");
    } else {
        logger.error("DobbyHook failed: {}", useItemOn_ret);
    }
}