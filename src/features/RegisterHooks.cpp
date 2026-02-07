#include "features/RegisterHooks.hpp"
#include <string>

class BaseGameVersion;
class Experiments;
namespace cereal { struct ReflectionCtx; }
class VanillaItems;

using TargetFn = void(*)(
        VanillaItems*,
        const cereal::ReflectionCtx&,
        const ItemRegistryRef,
        const BaseGameVersion&,
        const Experiments&
);

static TargetFn orig = nullptr;

void hook(
        VanillaItems* self,
        const cereal::ReflectionCtx& uk,
        const ItemRegistryRef itemRegistry,
        const BaseGameVersion& baseGameVersion,
        const Experiments& experiments
) {
    logger.info("Hook Function Called");
    logger.info("Calling Original Function...");
    orig(self, uk, itemRegistry, baseGameVersion, experiments);
    
    logger.info("trying patch item...");
    ItemRegistry* registry = itemRegistry._lockRegistry().get();
    for (auto& pair : registry->mIdToItemMap)
    {
        pair.second.get()->setAllowOffhand(true);
    }
    logger.info("Success!");
}

class ShulkerBoxBlockItem;
class ItemStackBase;

using Shulker_appendHover_t = void (*)(void*, ItemStackBase*, void*, std::string&, bool);

inline Shulker_appendHover_t ShulkerBoxBlockItem_appendFormattedHovertext_orig = nullptr;

inline void ShulkerBoxBlockItem_appendFormattedHovertext_hook(
    ShulkerBoxBlockItem* self,
    ItemStackBase* stack,
    void* level,
    std::string& out,
    bool flag)
{
    if (ShulkerBoxBlockItem_appendFormattedHovertext_orig) ShulkerBoxBlockItem_appendFormattedHovertext_orig(self, stack, level, out, flag);
    
    out += "\n Hooked!!!!!!";
}

void RegisterHooks() {
    HOOK(
        "FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 FF 07 40 D1 FF 83 00 D1 48 D0 3B D5 EA 83 00 B2",
        (void*)hook,
        (void**)&orig
    );
    VHOOK(
        "19ShulkerBoxBlockItem",
        53,
        ShulkerBoxBlockItem_appendFormattedHovertext_hook,
        ShulkerBoxBlockItem_appendFormattedHovertext_orig
    );
};