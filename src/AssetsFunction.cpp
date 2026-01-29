#include <stdio.h>
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