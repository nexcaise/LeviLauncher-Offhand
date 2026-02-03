#include "pl/Gloss.h"
#include <dobby.h>
#include "pl/Signature.h"
#include "pl/Logger.h"
//#include "pl/internal/macro.h"
#include "util/InteractionResult.hpp"
#include "world/item/ItemStack.hpp"
#include "world/item/Item.hpp"

pl::log::Logger omlogger("OffhandMod");

using useItemOnFn = InteractionResult* (*)(
        void*,
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
        bool isFirstEvent
) {
    Item* item = stack.getItem();
    bool realMayUse = stack.mItem == nullptr;//!stack.mValid || stack.mItem == nullptr || stack.isNull() ||/* stack.mCount == 0 || !isSimTick ||*/ !item/* || item->canUseOnSimTick()*/;

    if (!realMayUse)
    {
        omlogger.info("May not use item");
        return useItemOn_orig(self,stack,at,face,hit,tb,isFirstEvent);
    }
    
    omlogger.info("item.setAllowOffhand(true);");
    item->setAllowOffhand(true);

    return useItemOn_orig(self,stack,at,face,hit,tb,isFirstEvent);

}

__attribute__((constructor))
void Init() {
    GlossInit(true);
    uintptr_t useItemOn_addr = pl::signature::pl_resolve_signature(
        "EC 13 40 F9 E8 03 16 AA",
        "libminecraftpe.so"
    );
    if (!useItemOn_addr) {
        omlogger.error("Signature not found");
        return;
    }
    omlogger.info("Signature found at: {}", (void*)useItemOn_addr);
    int useItemOn_ret = DobbyHook(
        (void*)useItemOn_addr,
        (void*)useItemOn_hook,
        (void**)&useItemOn_orig
    );
    if (useItemOn_ret == 0) {
        omlogger.info("DobbyHook success");
    } else {
        omlogger.error("DobbyHook failed: {}", useItemOn_ret);
    }
}