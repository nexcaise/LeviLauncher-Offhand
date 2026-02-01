#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <unistd.h>
#include <cstring>
#include <string>

#include <dobby.h>

#include "pl/Signature.h"
#include "pl/Gloss.h"
#include "ItemRegistry.hpp"
#include "ItemRegistryRef.hpp"
#include "VanillaItems.hpp"

#define LOG_TAG "HmmAja"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class BaseGameVersion;
class Experiments;
namespace cereal { struct ReflectionCtx; }

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
    LOGI("Hook::Start");
    
    ItemRegistry* registry = itemRegistry.mWeakRegistry.lock().get();
    //ItemRegistry& reg = *registry;
    
    for (auto& pair : registry->mIdToItemMap)
    {
        LOGI("ns: %s", pair.second.get()->mNamespace.c_str());
        pair.second.get()->setAllowOffhand(true);
        //LOGI("rn: %s", pair.second.get()->mRawNameId.c_str());
    }
    //LOGI("Item Count: %d", itemRegistry.getItemCount());
    LOGI("Hook::End");
    orig(self, uk, itemRegistry, baseGameVersion, experiments);
}

__attribute__((constructor))
void Init() {
    GlossInit(true);

    uintptr_t addr = pl::signature::pl_resolve_signature(
        //1.21.132
        "FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 13 20 00 91 F5 03 00 AA"
        //"FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 F3 03 02 AA F4 03 01 AA 61 0A 80 52 22 00 80 52",
        //1.21.124
        //"FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 FF 07 40 D1 FF 83 00 D1 48 D0 3B D5 EA 83 00 B2",
        "libminecraftpe.so"
    );

    if (!addr) {
        LOGE("Signature not found");
        return;
    }

    LOGI("Signature found at: %p", (void*)addr);

    int ret = DobbyHook(
        (void*)addr,
        (void*)hook,
        (void**)&orig
    );

    if (ret == 0) {
        LOGI("DobbyHook success");
    } else {
        LOGE("DobbyHook failed: %d", ret);
    }
}