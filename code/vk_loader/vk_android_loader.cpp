#include <platform_detect.h>
#include <dlfcn.h>

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>

#define shared_function

#include "vk_loader.h"

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

static PFN_vkVoidFunction VK_Load_Android_Func(vk_loader* Loader, const char* Name); 

#define VK_Load_Global_Func(object, name) object.name = (PFN_##name)VK_Load_Android_Func(this, #name)
#define VK_Load_Instance_Func(object, name) object.name = (PFN_##name)vkGetInstanceProcAddr(Instance, #name)
#define VK_Load_Device_Func(object, name) object.name = (PFN_##name)vkGetInstanceProcAddr(Instance, #name)

#include "vk_shared_loader.cpp"

struct vk_android_loader : public vk_shared_loader {
    void*                     Library;
};

extern "C" shared_function vk_loader* VK_Get_Loader() {
    static vk_android_loader Result;
    if(!Result.Library) {
        Result.Library = dlopen("libvulkan.so", RTLD_NOW|RTLD_GLOBAL);
        if(!Result.Library) {
            return nullptr;
        }

        vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(Result.Library, "vkGetInstanceProcAddr");
    }
    return &Result;
}

static PFN_vkVoidFunction VK_Load_Android_Func(vk_loader* Loader, const char* Name) {
    vk_android_loader* AndroidLoader = (vk_android_loader*)Loader;
    return (PFN_vkVoidFunction)dlsym(AndroidLoader->Library, Name);
}