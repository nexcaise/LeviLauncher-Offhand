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

#define LOG_TAG "HmmAja"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class BaseGameVersion;
class Experiments;
namespace cereal { struct ReflectionCtx; }

static void (*orig)(
        VanillaItems*,
        void*,
		const ItemRegistryRef,
		const BaseGameVersion&,
		const Experiments&) = nullptr;

void hook(
		VanillaItems* self,
		void* uk,
		const ItemRegistryRef itemRegistry,
		const BaseGameVersion& baseGameVersion,
		const Experiments& experiments
	) {
	    LOGI("Hook::Running");
		orig(
			self,
			uk,
			itemRegistry,
			baseGameVersion,
			experiments
		);

		ItemRegistry* registry = itemRegistry._lockRegistry().get();

        for (auto& pair : registry->mIdToItemMap) {
            pair.second->setAllowOffhand(true);
        }
}

__attribute__((constructor))
void Init() {
    GlossInit(true);
    void* sym = (void*)GlossSymbol(GlossOpen("libminecraftpe.so"),
        "ZN12VanillaItems13registerItemsERN6cereal13ReflectionCtxE15ItemRegistryRefRK15BaseGameVersionRK11ExperimentsE3", nullptr);
    if (sym) {
        GHook h = GlossHook(sym, (void*)hook, (void**)&orig);
        if (h) {
            LOGE("Failed hook!");
            return;
        }
    }
}