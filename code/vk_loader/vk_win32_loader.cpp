#include <platform_detect.h>
#include <windows.h>

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define shared_function __declspec(dllexport)

#include "vk_loader.h"

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
static PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;

#define VK_Load_Global_Func(object, name) object.name = (PFN_##name)vkGetInstanceProcAddr(nullptr, #name)
#define VK_Load_Instance_Func(object, name) object.name = (PFN_##name)vkGetInstanceProcAddr(Instance, #name)
#define VK_Load_Device_Func(object, name) object.name = (PFN_##name)vkGetDeviceProcAddr(Device, #name)

#include "vk_shared_loader.cpp"

struct vk_win32_loader : public vk_shared_loader {
    HMODULE Library;
    virtual void Load_Instance_Funcs(VkInstance Instance) override;
};

void vk_win32_loader::Load_Instance_Funcs(VkInstance Instance) {
    vk_shared_loader::Load_Instance_Funcs(Instance);

    if(!vkGetDeviceProcAddr)
        vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(Instance, "vkGetDeviceProcAddr");
}

extern "C" shared_function vk_loader* VK_Get_Loader() {
    static vk_win32_loader Result;
    if(!Result.Library) {
        Result.Library = LoadLibraryA("vulkan-1.dll");
        if(!Result.Library) {
            return nullptr;
        }

        vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(Result.Library, "vkGetInstanceProcAddr");
    }
    return &Result;
}