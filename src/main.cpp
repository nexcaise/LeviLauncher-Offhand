#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <unistd.h>
#include <cstring>
#include <string>

#include <dobby.h>

#include "pl/Signature.h"
#include "pl/Gloss.h"
#include "util/ItemRegistry.hpp"
#include "util/ItemRegistryRef.hpp"
#include "util/VanillaItems.hpp"

#define LOG_TAG "HmmAja"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class BaseGameVersion;
class Experiments;
namespace cereal { struct ReflectionCtx; }

using TargetFn = void(*)(
        VanillaItems*,
        cereal::ReflectionCtx&,
        ItemRegistryRef,
        BaseGameVersion const&,
        Experiments const&
);

static TargetFn orig = nullptr;

void hook(
        VanillaItems* self,
        cereal::ReflectionCtx& ctx,
        ItemRegistryRef itemRegistry,
        BaseGameVersion const& baseGameVersion,
        Experiments const& experiments
) {
    LOGI("Hook::Start");

    if (orig) {
        orig(self, ctx, itemRegistry, baseGameVersion, experiments);
    }

    uint8_t* p = reinterpret_cast<uint8_t*>(&itemRegistry);
    for (int i = 0; i < 0x100; i += 8) {
        uint64_t v;
        memcpy(&v, p + i, sizeof(v));
        LOGI("+0x%02X : %016llX", i, v);
    }

    LOGI("Hook::End");
}

__attribute__((constructor))
void Init() {
    GlossInit(true);
    uintptr_t addr = pl::signature::pl_resolve_signature(
        //1.21.124
        //"FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 FF 07 40 D1 FF 83 00 D1 48 D0 3B D5 EA 83 00 B2"
        //1.21.132
        "FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 FF 07 40 D1 FF 03 01 D1 48 D0 3B D5"
        ,
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