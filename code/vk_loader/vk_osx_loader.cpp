#include <platform_detect.h>

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan.h>

#define shared_function

#include "vk_loader.h"


extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance, const char*);
static PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;

#define VK_Load_Global_Func(object, name) object.name = (PFN_##name)vkGetInstanceProcAddr(nullptr, #name)
#define VK_Load_Instance_Func(object, name) object.name = (PFN_##name)vkGetInstanceProcAddr(Instance, #name)
#define VK_Load_Device_Func(object, name) object.name = (PFN_##name)vkGetDeviceProcAddr(Device, #name)

#include "vk_shared_loader.cpp"

struct vk_osx_loader : public vk_shared_loader {
    virtual void Load_Instance_Funcs(VkInstance Instance) override;
};

void vk_osx_loader::Load_Instance_Funcs(VkInstance Instance) {
    vk_shared_loader::Load_Instance_Funcs(Instance);

    if(!vkGetDeviceProcAddr)
        vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(Instance, "vkGetDeviceProcAddr");
}

extern "C" shared_function vk_loader* VK_Get_Loader() {
    static vk_osx_loader Result;
    return &Result;
}