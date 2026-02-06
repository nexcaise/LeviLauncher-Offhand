#include "features/Hooks.hpp"

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

void RegisterHooks() {
    Hook(
        "FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 FF 07 40 D1 FF 83 00 D1 48 D0 3B D5 EA 83 00 B2",
        (void*)hook,
        (void**)&orig
    );
};