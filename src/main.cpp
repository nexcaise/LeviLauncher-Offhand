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

#define TAG "AssetsManager - Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

/*
std::string dataDir = "/storage/emulated/0/games/org.levimc";
std::vector<std::string> shadersList;
std::string assetsToRoot;*/

static AAsset* (*orig_AAssetManager_open)(AAssetManager*, const char*, int) = nullptr;

static AAsset* custom_AAssetManager_open(AAssetManager* mgr, const char* filename, int mode) {
    if ((strstr(filename, "title.png"))) {
        //std::string fName = std::string(filename).substr(std::string(filename).find_last_of("/") + 1);
        /*if (std::find(shadersList.begin(), shadersList.end(), fName) != shadersList.end()) {
            LOGI("Patched file %s via AAssetManager", fName.c_str());
            return orig_AAssetManager_open(mgr, (assetsToRoot + dataDir + "/shaders/" + fName).c_str(), mode); // uses custom asset path like /path/to/assets/../../../ to get to root
        } else {*/
            return orig_AAssetManager_open(mgr, /*filename*/"assets/resource_packs/vanilla/pack_icon.png", mode);
        //}
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
        return;
    } else {
        LOGI("Successfully = %d", r);
    }
}
/*
static int countCharacterOccurrences(const std::string& str, char character) {
    int count = 0;
    for (char c : str) {
        if (c == character) {
            count++;
        }
    }
    return count;
}*/

static void Setup() {/*
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
    }*/
    tryHook();
}

extern "C" __attribute__((visibility("default")))
void LeviMod_Load() {
    GlossInit(true);
    Setup();
}

__attribute__((constructor))
void AssetsManager_Init() {
    GlossInit(true);
    Setup();
}