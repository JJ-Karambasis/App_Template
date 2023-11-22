#include <platform_detect.h>

#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan.h>

#define shared_function

#include "vk_loader.h"

#define VK_Load_Global_Func(object, name) object.name = name
#define VK_Load_Instance_Func(object, name) object.name = name
#define VK_Load_Device_Func(object, name) object.name = name

#include "vk_shared_loader.cpp"

extern "C" shared_function vk_loader* VK_Get_Loader() {
    static vk_shared_loader Result;
    return &Result;
}