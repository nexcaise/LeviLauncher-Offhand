#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <dlfcn.h>
#include <unistd.h>

#include <cstring>
#include <string>

#include "pl/Hook.h"
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
    uintptr_t addr = pl::signature::pl_resolve_signature(
        "FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 FF 07 40 D1 FF 83 00 D1 48 D0 3B D5 EA 83 00 B2",
        //"FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 F3 03 02 AA F4 03 01 AA 61 0A 80 52 22 00 80 52",
        "libminecraftpe.so"
    );

    if (!addr) {
        LOGE("SignatureNot found!");
        return;
    }
    
    pl::hook::pl_hook((pl::hook::FuncPtr)addr, (pl::hook::FuncPtr)hook, (pl::hook::FuncPtr*)&orig, pl::hook::PriorityHighest);
}