#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <dlfcn.h>
#include <unistd.h>

#include <cstring>
#include <string>

#include "pl/Hook.h"
#include "pl/Gloss.h"
#include "ItemRegistry.hpp"
#include "ItemRegistryRef.hpp"
#include "VanillaItems.hpp"


#define TAG "AssetsManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

class BaseGameVersion;
class Experiments;
namespace cereal { struct ReflectionCtx; }

static void (*orig_AAssetManager_open)(
        VanillaItems*,
        void*,
		const ItemRegistryRef,
		const BaseGameVersion&,
		const Experiments&) = nullptr;

void VanillaItems_registerItems(
		VanillaItems* self,
		void* uk,
		const ItemRegistryRef itemRegistry,
		const BaseGameVersion& baseGameVersion,
		const Experiments& experiments
	) {
		orig_VanillaItems_registerItems(
			self,
			uk,
			itemRegistry,
			baseGameVersion,
			experiments
		);

		ItemRegistry* registry = itemRegistry._lockRegistry().get();
		for (auto& pair : registry.mIdToItemMap)
        {
            pair.second.get()->setAllowOffhand(true);
        }
}

static void tryHook() {
    void* lib = dlopen("libminecraftpe.so", RTLD_NOW);
    if (!lib) {
        LOGE("dlopen libminecraftpe.so failed");
        return;
    }

    void* sym = dlsym(lib, "_ZN12VanillaItems13registerItemsEb");
    if (!sym) {
        LOGE("dlsym AAssetManager_open failed");
        return;
    }

    int r = pl::hook::pl_hook((pl::hook::FuncPtr)sym, (pl::hook::FuncPtr)VanillaItems_registerItems, (pl::hook::FuncPtr*)&orig_AAssetManager_open, pl::hook::PriorityHighest);

    if(r != 0) {
        LOGE("Failed to hook!");
        return;
    }
}

extern "C" {
    __attribute__((visibility("default")))
    void LeviMod_Load(JavaVM* vm) {
        GlossInit(true);
        tryHook();
    }
}