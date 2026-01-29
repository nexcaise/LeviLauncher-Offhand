#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cstring>
#include <cstdint>
#include <sys/mman.h>
#include <unistd.h>
#include <fstream>
#include <string>

//#include "common/transition.h"
#include "pl/Gloss.h"

#define LOG_TAG "HmmAja"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static void (*g_Item_appendFormattedHovertext_orig)(
    void* /*this*/,
    void* /*ItemStackBase*/,
    void* /*Level*/,
    std::string* /*out*/,
    bool /*advanced*/
) = nullptr;

static void Item_appendFormattedHovertext_hook(
    void* thisPtr,
    void* itemStack,
    void* level,
    std::string* out,
    bool advanced
) {
    if (g_Item_appendFormattedHovertext_orig) {
        g_Item_appendFormattedHovertext_orig(
            thisPtr,
            itemStack,
            level,
            out,
            true/*advanced*/
        );
    }
    if (out) {
        out->append("\n§7[Hooked appendFormattedHovertext]");
    }
}

static bool findAndHookItemAppendHovertext() {
    void* mcLib = dlopen("libminecraftpe.so", RTLD_NOLOAD);
    if (!mcLib) mcLib = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!mcLib) {
        LOGE("Failed to open libminecraftpe.so");
        return false;
    }

    const char* typeinfoName = "19ShulkerBoxBlockItem";
    size_t nameLen = strlen(typeinfoName);

    uintptr_t typeinfoNameAddr = 0;
    uintptr_t typeinfoAddr = 0;
    uintptr_t vtableAddr = 0;

    std::ifstream maps("/proc/self/maps");
    std::string line;

    // 🔍 cari "4Item"
    while (std::getline(maps, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos &&
            line.find("r-xp") == std::string::npos) continue;

        uintptr_t start, end;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;

        for (uintptr_t addr = start; addr < end - nameLen; addr++) {
            if (memcmp((void*)addr, typeinfoName, nameLen) == 0) {
                typeinfoNameAddr = addr;
                LOGI("Found Item typeinfo name at 0x%lx", addr);
                break;
            }
        }
        if (typeinfoNameAddr) break;
    }

    if (!typeinfoNameAddr) {
        LOGE("Failed to find Item typeinfo name");
        return false;
    }

    // 🔍 cari typeinfo
    std::ifstream maps2("/proc/self/maps");
    while (std::getline(maps2, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos) continue;

        uintptr_t start, end;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;

        for (uintptr_t addr = start; addr < end - sizeof(void*); addr += sizeof(void*)) {
            if (*(uintptr_t*)addr == typeinfoNameAddr) {
                typeinfoAddr = addr - sizeof(void*);
                LOGI("Found Item typeinfo at 0x%lx", typeinfoAddr);
                break;
            }
        }
        if (typeinfoAddr) break;
    }

    if (!typeinfoAddr) {
        LOGE("Failed to find Item typeinfo");
        return false;
    }

    // 🔍 cari vtable
    std::ifstream maps3("/proc/self/maps");
    while (std::getline(maps3, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos) continue;

        uintptr_t start, end;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;

        for (uintptr_t addr = start; addr < end - sizeof(void*); addr += sizeof(void*)) {
            if (*(uintptr_t*)addr == typeinfoAddr) {
                vtableAddr = addr + sizeof(void*);
                LOGI("Found Item vtable at 0x%lx", vtableAddr);
                break;
            }
        }
        if (vtableAddr) break;
    }

    if (!vtableAddr) {
        LOGE("Failed to find Item vtable");
        return false;
    }

    // 🎯 index 54
    uintptr_t* slot = (uintptr_t*)(vtableAddr + 48 * sizeof(void*));
    g_Item_appendFormattedHovertext_orig =
        (decltype(g_Item_appendFormattedHovertext_orig))(*slot);

    LOGI("Original appendFormattedHovertext at 0x%lx",
         (uintptr_t)g_Item_appendFormattedHovertext_orig);

    // 🛠️ patch vtable
    uintptr_t page = (uintptr_t)slot & ~(4095UL);
    mprotect((void*)page, 4096, PROT_READ | PROT_WRITE);
    *slot = (uintptr_t)&Item_appendFormattedHovertext_hook;
    mprotect((void*)page, 4096, PROT_READ);

    LOGI("Successfully hooked Item::appendFormattedHovertext");
    return true;
}
/*
extern "C" __attribute__((visibility("default")))
void LeviMod_Load() {
    GlossInit(true);
    if(!findAndHookItemAppendHovertext()) LOGE("Error Cant hook!");
    //Setup();
}*/

__attribute__((constructor))
void /*AssetsManager_*/Init() {
    GlossInit(true);
    if(!findAndHookItemAppendHovertext()) LOGE("Error Cant hook!");
    //Setup();
}