#ifndef VK_LOADER_H
#define VK_LOADER_H

#define VK_FUNCTION(name) PFN_##name name

struct vk_khr_surface {
    bool Enabled;
    VK_FUNCTION(vkDestroySurfaceKHR);
    VK_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
    VK_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);
};

struct vk_khr_get_physical_device_properties2 {
    bool Enabled;
    VK_FUNCTION(vkGetPhysicalDeviceFeatures2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceProperties2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceFormatProperties2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceImageFormatProperties2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceMemoryProperties2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceSparseImageFormatProperties2KHR);
};

struct vk_ext_debug_report {
    bool Enabled;
    VK_FUNCTION(vkCreateDebugReportCallbackEXT);
    VK_FUNCTION(vkDebugReportMessageEXT);
    VK_FUNCTION(vkDestroyDebugReportCallbackEXT);
};

struct vk_ext_debug_utils {
    bool Enabled;
    VK_FUNCTION(vkSetDebugUtilsObjectNameEXT);
    VK_FUNCTION(vkSetDebugUtilsObjectTagEXT);
    VK_FUNCTION(vkQueueBeginDebugUtilsLabelEXT);
    VK_FUNCTION(vkQueueEndDebugUtilsLabelEXT);
    VK_FUNCTION(vkQueueInsertDebugUtilsLabelEXT);
    VK_FUNCTION(vkCmdBeginDebugUtilsLabelEXT);
    VK_FUNCTION(vkCmdEndDebugUtilsLabelEXT);
    VK_FUNCTION(vkCmdInsertDebugUtilsLabelEXT);
    VK_FUNCTION(vkCreateDebugUtilsMessengerEXT);
    VK_FUNCTION(vkDestroyDebugUtilsMessengerEXT);
    VK_FUNCTION(vkSubmitDebugUtilsMessageEXT);
};

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct vk_khr_win32_surface {
    bool Enabled;
    VK_FUNCTION(vkCreateWin32SurfaceKHR);
};
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
struct vk_khr_android_surface {
    bool Enabled;
    VK_FUNCTION(vkCreateAndroidSurfaceKHR);
};
#elif defined(VK_USE_PLATFORM_METAL_EXT)
struct vk_ext_metal_surface {
    bool Enabled;
    VK_FUNCTION(vkCreateMetalSurfaceEXT);
};
#else
#error Not Implemented
#endif

struct vk_khr_swapchain {
    bool Enabled;
    VK_FUNCTION(vkCreateSwapchainKHR);
    VK_FUNCTION(vkDestroySwapchainKHR);
    VK_FUNCTION(vkGetSwapchainImagesKHR);
    VK_FUNCTION(vkAcquireNextImageKHR);
    VK_FUNCTION(vkQueuePresentKHR);
};

struct vk_global_funcs {
    VK_FUNCTION(vkEnumerateInstanceExtensionProperties);
    VK_FUNCTION(vkEnumerateInstanceLayerProperties);
    VK_FUNCTION(vkCreateInstance);
};

struct vk_instance_funcs {
    VK_FUNCTION(vkEnumeratePhysicalDevices);
    VK_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
    VK_FUNCTION(vkEnumerateDeviceExtensionProperties);
    VK_FUNCTION(vkCreateDevice);
    VK_FUNCTION(vkGetPhysicalDeviceProperties);
    VK_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
    VK_FUNCTION(vkGetDeviceQueue);

    vk_khr_surface SurfaceKHR;
    vk_khr_get_physical_device_properties2 GetPhysicalDeviceProperties2KHR;
    vk_ext_debug_utils  DebugUtilsEXT;
    vk_ext_debug_report DebugReportEXT;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    vk_khr_win32_surface Win32SurfaceKHR;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    vk_khr_android_surface AndroidSurfaceKHR;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    vk_ext_metal_surface MetalSurfaceEXT;
#else
#error Not Implemented
#endif
};

struct vk_device_funcs {
    VK_FUNCTION(vkCreateCommandPool);
    VK_FUNCTION(vkAllocateCommandBuffers);
    VK_FUNCTION(vkCreateFence);
    VK_FUNCTION(vkCreateSemaphore);
    VK_FUNCTION(vkResetFences);
    VK_FUNCTION(vkResetCommandPool);
    VK_FUNCTION(vkBeginCommandBuffer);
    VK_FUNCTION(vkEndCommandBuffer);
    VK_FUNCTION(vkQueueSubmit);
    VK_FUNCTION(vkGetFenceStatus);
    VK_FUNCTION(vkWaitForFences);
    VK_FUNCTION(vkCreateRenderPass);
    VK_FUNCTION(vkCreateImageView);
    VK_FUNCTION(vkCreateFramebuffer);
    VK_FUNCTION(vkCmdPipelineBarrier);
    VK_FUNCTION(vkCmdBeginRenderPass);
    VK_FUNCTION(vkCmdEndRenderPass);
    VK_FUNCTION(vkCreateBuffer);
    VK_FUNCTION(vkGetBufferMemoryRequirements);
    VK_FUNCTION(vkAllocateMemory);
    VK_FUNCTION(vkBindBufferMemory);
    VK_FUNCTION(vkMapMemory);
    VK_FUNCTION(vkCmdCopyBuffer);
    VK_FUNCTION(vkCreatePipelineLayout);
    VK_FUNCTION(vkCreateShaderModule);
    VK_FUNCTION(vkCreateGraphicsPipelines);
    VK_FUNCTION(vkCmdSetViewport);
    VK_FUNCTION(vkCmdSetScissor);
    VK_FUNCTION(vkCmdBindPipeline);
    VK_FUNCTION(vkCmdBindVertexBuffers);
    VK_FUNCTION(vkCmdBindIndexBuffer);
    VK_FUNCTION(vkCmdDrawIndexed);

    vk_khr_swapchain SwapchainKHR;
};

struct vk_instance_extension_support {
    bool SurfaceKHR;
    bool GetPhysicalDeviceProperties2KHR;
    bool DebugUtilsEXT;
    bool DebugReportEXT;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    bool Win32SurfaceKHR;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    bool AndroidSurfaceKHR;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    bool MetalSurfaceEXT;
#else
    #error Not Implemented
#endif
};

struct vk_loader_instance_info {
    VkInstance                    Instance;
    vk_instance_extension_support Extensions;
};

struct vk_device_extension_support {
    bool SwapchainKHR;
};

struct vk_loader_device_info {
    VkInstance                  Instance;
    VkDevice                    Device;
    vk_device_extension_support Extensions;
};

struct vk_loader {
    virtual const vk_global_funcs*   Load_Global_Funcs() = 0;
    virtual const vk_instance_funcs* Load_Instance_Funcs(const vk_loader_instance_info& InstanceInfo) = 0;
    virtual const vk_device_funcs*   Load_Device_Funcs(const vk_loader_device_info& DeviceInfo) = 0;
};

extern "C" shared_function vk_loader* VK_Get_Loader();

#endif