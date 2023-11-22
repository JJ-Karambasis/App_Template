struct vk_shared_loader : public vk_loader {
    vk_global_funcs         GlobalFuncs;
    vk_instance_funcs       InstanceFuncs;
    vk_device_funcs         DeviceFuncs;
    bool                    HasLoadedGlobals;
    bool                    HasLoadedInstances;
    bool                    HasLoadedDevices;

    virtual void Load_Instance_Funcs(VkInstance Instance);
    void Load_Surface_KHR_Funcs(VkInstance Instance);
    void Load_Get_Physical_Device_Properties_2_KHR_Funcs(VkInstance Instance);
    void Load_Debug_Report_EXT_Funcs(VkInstance Instance);
    void Load_Debug_Utils_EXT_Funcs(VkInstance Instance);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    void Load_Win32_Surface_KHR_Funcs(VkInstance Instance);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    void Load_Android_Surface_KHR_Funcs(VkInstance Instance);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    void Load_Metal_Surface_EXT_Funcs(VkInstance Instance);
#else
#error Not Implemented
#endif

    void Load_Device_Funcs(VkInstance Instance, VkDevice Device);
    void Load_Swapchain_KHR_Funcs(VkInstance Instance, VkDevice Device);

    const vk_global_funcs*   Load_Global_Funcs() final;
    const vk_instance_funcs* Load_Instance_Funcs(const vk_loader_instance_info& InstanceInfo) final;
    const vk_device_funcs*   Load_Device_Funcs(const vk_loader_device_info& DeviceInfo) final;
};

void vk_shared_loader::Load_Instance_Funcs(VkInstance Instance) {
    if(!HasLoadedInstances) {
        VK_Load_Instance_Func(InstanceFuncs, vkEnumeratePhysicalDevices);
        VK_Load_Instance_Func(InstanceFuncs, vkGetPhysicalDeviceQueueFamilyProperties);
        VK_Load_Instance_Func(InstanceFuncs, vkEnumerateDeviceExtensionProperties);
        VK_Load_Instance_Func(InstanceFuncs, vkCreateDevice);
        VK_Load_Instance_Func(InstanceFuncs, vkGetPhysicalDeviceProperties);
        VK_Load_Instance_Func(InstanceFuncs, vkGetPhysicalDeviceMemoryProperties);
        VK_Load_Instance_Func(InstanceFuncs, vkGetDeviceQueue);
        HasLoadedInstances = true;
    }
}

void vk_shared_loader::Load_Surface_KHR_Funcs(VkInstance Instance) {
    if(!InstanceFuncs.SurfaceKHR.Enabled) {
        VK_Load_Instance_Func(InstanceFuncs.SurfaceKHR, vkDestroySurfaceKHR);
        VK_Load_Instance_Func(InstanceFuncs.SurfaceKHR, vkGetPhysicalDeviceSurfaceSupportKHR);
        VK_Load_Instance_Func(InstanceFuncs.SurfaceKHR, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
        VK_Load_Instance_Func(InstanceFuncs.SurfaceKHR, vkGetPhysicalDeviceSurfaceFormatsKHR);
        VK_Load_Instance_Func(InstanceFuncs.SurfaceKHR, vkGetPhysicalDeviceSurfacePresentModesKHR);
        InstanceFuncs.SurfaceKHR.Enabled = true;
    }
}

void vk_shared_loader::Load_Get_Physical_Device_Properties_2_KHR_Funcs(VkInstance Instance) {
    if(!InstanceFuncs.GetPhysicalDeviceProperties2KHR.Enabled) {
        VK_Load_Instance_Func(InstanceFuncs.GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceFeatures2KHR);
        VK_Load_Instance_Func(InstanceFuncs.GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceProperties2KHR);
        VK_Load_Instance_Func(InstanceFuncs.GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceFormatProperties2KHR);
        VK_Load_Instance_Func(InstanceFuncs.GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceImageFormatProperties2KHR);
        VK_Load_Instance_Func(InstanceFuncs.GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceQueueFamilyProperties2KHR);
        VK_Load_Instance_Func(InstanceFuncs.GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceMemoryProperties2KHR);
        VK_Load_Instance_Func(InstanceFuncs.GetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceSparseImageFormatProperties2KHR);
        InstanceFuncs.GetPhysicalDeviceProperties2KHR.Enabled = true;
    }
}

void vk_shared_loader::Load_Debug_Report_EXT_Funcs(VkInstance Instance) {
    if(!InstanceFuncs.DebugReportEXT.Enabled) {
        VK_Load_Instance_Func(InstanceFuncs.DebugReportEXT, vkCreateDebugReportCallbackEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugReportEXT, vkDebugReportMessageEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugReportEXT, vkDestroyDebugReportCallbackEXT);
        InstanceFuncs.DebugReportEXT.Enabled = true;
    }
}

void vk_shared_loader::Load_Debug_Utils_EXT_Funcs(VkInstance Instance) {
    if(!InstanceFuncs.DebugUtilsEXT.Enabled) {
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkSetDebugUtilsObjectNameEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkSetDebugUtilsObjectTagEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkQueueBeginDebugUtilsLabelEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkQueueEndDebugUtilsLabelEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkQueueInsertDebugUtilsLabelEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkCmdBeginDebugUtilsLabelEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkCmdEndDebugUtilsLabelEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkCmdInsertDebugUtilsLabelEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkCreateDebugUtilsMessengerEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkDestroyDebugUtilsMessengerEXT);
        VK_Load_Instance_Func(InstanceFuncs.DebugUtilsEXT, vkSubmitDebugUtilsMessageEXT);
        InstanceFuncs.DebugUtilsEXT.Enabled = true;
    }
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
void vk_shared_loader::Load_Win32_Surface_KHR_Funcs(VkInstance Instance) {
    if(!InstanceFuncs.Win32SurfaceKHR.Enabled) {
        VK_Load_Instance_Func(InstanceFuncs.Win32SurfaceKHR, vkCreateWin32SurfaceKHR);
        InstanceFuncs.Win32SurfaceKHR.Enabled = true;
    }
}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
void vk_shared_loader::Load_Android_Surface_KHR_Funcs(VkInstance Instance) {
    if(!InstanceFuncs.AndroidSurfaceKHR.Enabled) {
        VK_Load_Instance_Func(InstanceFuncs.AndroidSurfaceKHR, vkCreateAndroidSurfaceKHR);
        InstanceFuncs.AndroidSurfaceKHR.Enabled = true;
    }
}
#elif defined(VK_USE_PLATFORM_METAL_EXT)
void vk_shared_loader::Load_Metal_Surface_EXT_Funcs(VkInstance Instance) {
    if(!InstanceFuncs.MetalSurfaceEXT.Enabled) {
        VK_Load_Instance_Func(InstanceFuncs.MetalSurfaceEXT, vkCreateMetalSurfaceEXT);
        InstanceFuncs.MetalSurfaceEXT.Enabled = true;
    }
}
#else
#error Not Implemented
#endif

void vk_shared_loader::Load_Device_Funcs(VkInstance Instance, VkDevice Device) {
    if(!HasLoadedDevices) {
        VK_Load_Device_Func(DeviceFuncs, vkCreateCommandPool);
        VK_Load_Device_Func(DeviceFuncs, vkAllocateCommandBuffers);
        VK_Load_Device_Func(DeviceFuncs, vkCreateFence);
        VK_Load_Device_Func(DeviceFuncs, vkCreateSemaphore);
        VK_Load_Device_Func(DeviceFuncs, vkResetFences);
        VK_Load_Device_Func(DeviceFuncs, vkResetCommandPool);
        VK_Load_Device_Func(DeviceFuncs, vkBeginCommandBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkEndCommandBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkQueueSubmit);
        VK_Load_Device_Func(DeviceFuncs, vkGetFenceStatus);
        VK_Load_Device_Func(DeviceFuncs, vkWaitForFences);
        VK_Load_Device_Func(DeviceFuncs, vkCreateRenderPass);
        VK_Load_Device_Func(DeviceFuncs, vkCreateImageView);
        VK_Load_Device_Func(DeviceFuncs, vkCreateFramebuffer);
        VK_Load_Device_Func(DeviceFuncs, vkCmdPipelineBarrier);
        VK_Load_Device_Func(DeviceFuncs, vkCmdBeginRenderPass);
        VK_Load_Device_Func(DeviceFuncs, vkCmdEndRenderPass);
        VK_Load_Device_Func(DeviceFuncs, vkCreateBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkGetBufferMemoryRequirements);
        VK_Load_Device_Func(DeviceFuncs, vkAllocateMemory);
        VK_Load_Device_Func(DeviceFuncs, vkBindBufferMemory);
        VK_Load_Device_Func(DeviceFuncs, vkMapMemory);
        VK_Load_Device_Func(DeviceFuncs, vkCmdCopyBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkCreatePipelineLayout);
        VK_Load_Device_Func(DeviceFuncs, vkCreateShaderModule);
        VK_Load_Device_Func(DeviceFuncs, vkCreateGraphicsPipelines);
        VK_Load_Device_Func(DeviceFuncs, vkCmdSetViewport);
        VK_Load_Device_Func(DeviceFuncs, vkCmdSetScissor);
        VK_Load_Device_Func(DeviceFuncs, vkCmdBindPipeline);
        VK_Load_Device_Func(DeviceFuncs, vkCmdBindVertexBuffers);
        VK_Load_Device_Func(DeviceFuncs, vkCmdBindIndexBuffer);
        VK_Load_Device_Func(DeviceFuncs, vkCmdDrawIndexed);
        HasLoadedDevices = true;
    }
}

void vk_shared_loader::Load_Swapchain_KHR_Funcs(VkInstance Instance, VkDevice Device) {
    if(!DeviceFuncs.SwapchainKHR.Enabled) {
        VK_Load_Device_Func(DeviceFuncs.SwapchainKHR, vkCreateSwapchainKHR);
        VK_Load_Device_Func(DeviceFuncs.SwapchainKHR, vkDestroySwapchainKHR);
        VK_Load_Device_Func(DeviceFuncs.SwapchainKHR, vkGetSwapchainImagesKHR);
        VK_Load_Device_Func(DeviceFuncs.SwapchainKHR, vkAcquireNextImageKHR);
        VK_Load_Device_Func(DeviceFuncs.SwapchainKHR, vkQueuePresentKHR);
        DeviceFuncs.SwapchainKHR.Enabled = true;
    }
}

const vk_global_funcs* vk_shared_loader::Load_Global_Funcs() {
    if(!HasLoadedGlobals) {
        VK_Load_Global_Func(GlobalFuncs, vkEnumerateInstanceExtensionProperties);
        VK_Load_Global_Func(GlobalFuncs, vkEnumerateInstanceLayerProperties);
        VK_Load_Global_Func(GlobalFuncs, vkCreateInstance);
        HasLoadedGlobals = true;
    }

    return &GlobalFuncs;
}

const vk_instance_funcs* vk_shared_loader::Load_Instance_Funcs(const vk_loader_instance_info& InstanceInfo) {
    if(!HasLoadedInstances) {
        Load_Instance_Funcs(InstanceInfo.Instance);
    }

    if(InstanceInfo.Extensions.SurfaceKHR) {
        Load_Surface_KHR_Funcs(InstanceInfo.Instance);
    }

    if(InstanceInfo.Extensions.GetPhysicalDeviceProperties2KHR) {
        Load_Get_Physical_Device_Properties_2_KHR_Funcs(InstanceInfo.Instance);
    }

    if(InstanceInfo.Extensions.DebugReportEXT) {
        Load_Debug_Report_EXT_Funcs(InstanceInfo.Instance);
    }

    if(InstanceInfo.Extensions.DebugUtilsEXT) {
        Load_Debug_Utils_EXT_Funcs(InstanceInfo.Instance);
    }

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if(InstanceInfo.Extensions.Win32SurfaceKHR) {
        Load_Win32_Surface_KHR_Funcs(InstanceInfo.Instance);
    }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    if(InstanceInfo.Extensions.AndroidSurfaceKHR) {
        Load_Android_Surface_KHR_Funcs(InstanceInfo.Instance);
    }
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    if(InstanceInfo.Extensions.MetalSurfaceEXT) {
        Load_Metal_Surface_EXT_Funcs(InstanceInfo.Instance);
    }
#else
#error Not Implemented
#endif

    return &InstanceFuncs;
}

const vk_device_funcs* vk_shared_loader::Load_Device_Funcs(const vk_loader_device_info& DeviceInfo) {
    if(!HasLoadedDevices) {
        Load_Device_Funcs(DeviceInfo.Instance, DeviceInfo.Device);
    }

    if(DeviceInfo.Extensions.SwapchainKHR) {
        Load_Swapchain_KHR_Funcs(DeviceInfo.Instance, DeviceInfo.Device);
    }

    return &DeviceFuncs;
}