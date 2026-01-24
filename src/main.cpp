/*#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>

#include <string>
#include <cstring>

#include <iostream>
#include <vector>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#include <jni.h>
#include <android/log.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "pl/Hook.h"
#include "pl/Gloss.h"
#include "pl/Signature.h"

#include <functional>

#define TAG "Shaders Loader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

namespace Core {
    template <typename T>
    class PathBuffer;
}

namespace Bedrock {
    template <typename T>
    class NotNullNonOwnerPtr;
}

class IContentTierManager;
class ResourcePackManager;

ResourcePackManager* rpm = nullptr;

using ResourcePackManager_ctor_t = void (*)(ResourcePackManager* _this, std::function<Core::PathBuffer<std::string>()> getPath, const Bedrock::NotNullNonOwnerPtr<const IContentTierManager>& contentTierManager, bool needsToInitialize);

static ResourcePackManager_ctor_t rpm_orig = nullptr;

static void rpm_hook(ResourcePackManager* _this, std::function<Core::PathBuffer<std::string>()> getPath, const Bedrock::NotNullNonOwnerPtr<const IContentTierManager>& contentTierManager, bool needsToInitialize) {
    ResourcePackManager* thisPtr = _this;

    if(needsToInitialize) {
        rpm = thisPtr;
    }
    
    return rpm_orig(_this, getPath, contentTierManager, needsToInitialize);
}

static void findAndHook() {
    uintptr_t addr = pl::signature::pl_resolve_signature(
        "FF ? 02 D1 FD 7B ? A9 ? ? ? ? FA 67 ? A9 F8 5F ? A9 F6 57 ? A9 F4 4F ? A9 FD ? 01 91 ? D0 3B D5 ? 03 03 2A ? 03 02 AA ? 17 40 F9 F3 03 00 AA A8 83 1F F8",
        "libminecraftpe.so"
    );

    if (!addr) {
        LOGE("Signature Not found!");
        return;
    }
    
    LOGI("Signature addr: %p", (void*)addr);
    
    int r = pl::hook::pl_hook((pl::hook::FuncPtr)addr, (pl::hook::FuncPtr)gtw_hook, (pl::hook::FuncPtr*)&gtw_orig, pl::hook::PriorityHighest);
    
    if(r != 0) {
        LOGE("Failed to Hook = %d", r);
        return;
    } else {
        LOGI("Successfully = %d", r);
    }
}

std::string dataDir = "/storage/emulated/0/games";
std::vector<std::string> shadersList;

int countCharacterOccurrences(const std::string& str, char character) {
    int count = 0;
    for (char c : str) {
        if (c == character) {
            count++;
        }
    }
    return count;
}

std::string assetsToRoot;

static AAsset* (*orig_AAssetManager_open)(AAssetManager*, const char*, int) = nullptr;

static AAsset* custom_AAssetManager_open(AAssetManager* mgr, const char* filename, int mode) {
    if ((strstr(filename, ".material.bin"))) {
            std::string fName = std::string(filename).substr(std::string(filename).find_last_of("/") + 1);
            if (std::find(shadersList.begin(), shadersList.end(), fName) != shadersList.end()) {
            LOGI("Patched shader %s via AAssetManager", fName.c_str());
            return orig_AAssetManager_open(mgr, (assetsToRoot + dataDir + "/shaders/" + fName).c_str(), mode);
        } else {
            return orig_AAssetManager_open(mgr, filename, mode);
        }
    } else {
        return orig_AAssetManager_open(mgr, filename, mode);
    }
}

static void tryHook() {
    void* lib = dlopen("libminecraftpe.so", RTLD_NOLOAD);
    if (!lib) lib = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!lib) {
        LOGE("dlopen library failed");
        return;
    }

    void* sym = dlsym(lib, "AAssetManager_open");
    if (!sym) {
        LOGE("dlsym AAssetManager_open failed");
        return;
    }

    int r = pl::hook::pl_hook((pl::hook::FuncPtr)sym, (pl::hook::FuncPtr)custom_AAssetManager_open, (pl::hook::FuncPtr*)&orig_AAssetManager_open, pl::hook::PriorityHighest);
    
    if(r != 0) {
        LOGE("Failed to Hook = %d", r);
    } else {
        LOGI("Successfully = %d", r);
        findAndHook();
    }
}

static void Setup() {
    std::string assetsDir;

    char originalDir[PATH_MAX];
    getcwd(originalDir, sizeof(originalDir));
    chdir(dataDir.c_str());
    char currentDir[PATH_MAX];
    getcwd(currentDir, sizeof(currentDir));
    chdir(".");
    char currentDir2[PATH_MAX];
    getcwd(currentDir2, sizeof(currentDir2));
    chdir(originalDir);
    dataDir = currentDir;
    assetsDir = currentDir2;
    chdir("/");

    int occurrences = countCharacterOccurrences(assetsDir, '/');

    for(int i = 0; i<occurrences; i++) {
        assetsToRoot+="../";
    }
    
    DIR *dr;
    struct dirent *en;
    dr = opendir((dataDir + "/shaders").c_str());
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            if (strstr(en->d_name, ".material.bin")) {
                shadersList.push_back(std::string(en->d_name));
            }
        }
        closedir(dr);
    }
    
    tryHook();
}
*/

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
    if (out) {
        out->append("\n§7[Hooked appendFormattedHovertext]");
    }
    if (g_Item_appendFormattedHovertext_orig) {
        g_Item_appendFormattedHovertext_orig(
            thisPtr,
            itemStack,
            level,
            out,
            true/*advanced*/
        );
    }
}

static bool findAndHookItemAppendHovertext() {
    void* mcLib = dlopen("libminecraftpe.so", RTLD_NOLOAD);
    if (!mcLib) mcLib = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!mcLib) {
        LOGE("Failed to open libminecraftpe.so");
        return false;
    }

    const char* typeinfoName = "4Item";
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
    uintptr_t* slot = (uintptr_t*)(vtableAddr + 54 * sizeof(void*));
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

extern "C" __attribute__((visibility("default")))
void LeviMod_Load() {
    GlossInit(true);
    if(!findAndHookItemAppendHovertext()) LOGE("Error Cant hook!");
    //Setup();
}

__attribute__((constructor))
void AssetsManager_Init() {
    GlossInit(true);
    if(!findAndHookItemAppendHovertext()) LOGE("Error Cant hook!");
    //Setup();
}