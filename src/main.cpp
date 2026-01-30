#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cstring>
#include <cstdint>
#include <sys/mman.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include "util/ItemStackBase.h"

#include "pl/Gloss.h"

#define LOG_TAG "HmmAja"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class ShulkerBoxBlockItem;

using Shulker_appendHover_t = void (*)(void*, ItemStackBase*, void*, std::string&, bool);

static Shulker_appendHover_t g_ShulkerBoxBlockItem_appendFormattedHovertext_orig = nullptr;

static void ShulkerBoxBlockItem_appendFormattedHovertext_hook(
    ShulkerBoxBlockItem* self,
    ItemStackBase* stack,
    void* level,
    std::string& out,
    bool flag)
{
    if (g_ShulkerBoxBlockItem_appendFormattedHovertext_orig) g_ShulkerBoxBlockItem_appendFormattedHovertext_orig(self, stack, level, out, flag);

    out.append("\n§7[Hooked appendFormattedHovertext]");
}

static bool findAndHookShulkerBoxBlockItem() {
    void* mcLib = dlopen("libminecraftpe.so", RTLD_NOLOAD);
    if (!mcLib) mcLib = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!mcLib) return false;

    uintptr_t libBase = 0;
    std::ifstream maps("/proc/self/maps");
    std::string line;

    while (std::getline(maps, line)) {
        if (line.find("libminecraftpe.so") != std::string::npos &&
            line.find("r-xp") != std::string::npos) {
            sscanf(line.c_str(), "%lx-%*lx", &libBase);
            break;
        }
    }

    if (!libBase) return false;

    const char* typeinfoName = "19ShulkerBoxBlockItem";
    size_t nameLen = strlen(typeinfoName);
    uintptr_t typeinfoNameAddr = 0;

    std::ifstream maps2("/proc/self/maps");
    while (std::getline(maps2, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos &&
            line.find("r-xp") == std::string::npos) continue;

        uintptr_t start, end;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;

        for (uintptr_t addr = start; addr < end - nameLen; addr++) {
            if (memcmp((void*)addr, typeinfoName, nameLen) == 0) {
                typeinfoNameAddr = addr;
                break;
            }
        }
        if (typeinfoNameAddr) break;
    }

    if (!typeinfoNameAddr) return false;

    uintptr_t typeinfoAddr = 0;
    std::ifstream maps3("/proc/self/maps");

    while (std::getline(maps3, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos) continue;

        uintptr_t start, end;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;

        for (uintptr_t addr = start; addr < end - sizeof(void*); addr += sizeof(void*)) {
            if (*(uintptr_t*)addr == typeinfoNameAddr) {
                typeinfoAddr = addr - sizeof(void*);
                break;
            }
        }
        if (typeinfoAddr) break;
    }

    if (!typeinfoAddr) return false;

    uintptr_t vtableAddr = 0;
    std::ifstream maps4("/proc/self/maps");

    while (std::getline(maps4, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos) continue;

        uintptr_t start, end;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;

        for (uintptr_t addr = start; addr < end - sizeof(void*); addr += sizeof(void*)) {
            if (*(uintptr_t*)addr == typeinfoAddr) {
                vtableAddr = addr + sizeof(void*);
                break;
            }
        }
        if (vtableAddr) break;
    }

    if (!vtableAddr) return false;

    uintptr_t* slot53 = (uintptr_t*)(vtableAddr + 53 * sizeof(void*));

    g_ShulkerBoxBlockItem_appendFormattedHovertext_orig =
        (Shulker_appendHover_t)(*slot53);

    uintptr_t pageStart = ((uintptr_t)slot53) & ~(4095UL);
    if (mprotect((void*)pageStart, 4096, PROT_READ | PROT_WRITE) != 0)
        return false;

    *slot53 = (uintptr_t)&ShulkerBoxBlockItem_appendFormattedHovertext_hook;

    mprotect((void*)pageStart, 4096, PROT_READ);
    return true;
}

__attribute__((constructor))
void Init() {
    GlossInit(true);
    if(!findAndHookShulkerBoxBlockItem()) LOGE("Error Cant hook!");
}