void VK_Load_Global_Funcs(vk_loader* Loader) {
    const vk_global_funcs* GlobalFuncs = Loader->Load_Global_Funcs();

    vkEnumerateInstanceExtensionProperties = GlobalFuncs->vkEnumerateInstanceExtensionProperties;
    vkEnumerateInstanceLayerProperties = GlobalFuncs->vkEnumerateInstanceLayerProperties;
    vkCreateInstance = GlobalFuncs->vkCreateInstance;
}

void VK_Load_Instance_Funcs(vk_loader* Loader, const vk_loader_instance_info& InstanceInfo) {
    const vk_instance_funcs* InstanceFuncs = Loader->Load_Instance_Funcs(InstanceInfo);

    vkEnumeratePhysicalDevices = InstanceFuncs->vkEnumeratePhysicalDevices;
    vkGetPhysicalDeviceQueueFamilyProperties = InstanceFuncs->vkGetPhysicalDeviceQueueFamilyProperties;
    vkEnumerateDeviceExtensionProperties = InstanceFuncs->vkEnumerateDeviceExtensionProperties;
    vkCreateDevice = InstanceFuncs->vkCreateDevice;
    vkGetPhysicalDeviceProperties = InstanceFuncs->vkGetPhysicalDeviceProperties;
    vkGetPhysicalDeviceMemoryProperties = InstanceFuncs->vkGetPhysicalDeviceMemoryProperties;
    vkGetDeviceQueue = InstanceFuncs->vkGetDeviceQueue;

    Assert(InstanceFuncs->SurfaceKHR.Enabled, "VK_KHR_surface is not available!");
    vkGetPhysicalDeviceSurfaceSupportKHR = InstanceFuncs->SurfaceKHR.vkGetPhysicalDeviceSurfaceSupportKHR;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = InstanceFuncs->SurfaceKHR.vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    vkGetPhysicalDeviceSurfaceFormatsKHR = InstanceFuncs->SurfaceKHR.vkGetPhysicalDeviceSurfaceFormatsKHR;
}

void VK_Load_Device_Funcs(vk_loader* Loader, const vk_loader_device_info& DeviceInfo) {
    const vk_device_funcs* DeviceFuncs = Loader->Load_Device_Funcs(DeviceInfo);

    vkCreateCommandPool = DeviceFuncs->vkCreateCommandPool;
    vkAllocateCommandBuffers = DeviceFuncs->vkAllocateCommandBuffers;
    vkCreateFence = DeviceFuncs->vkCreateFence;
    vkCreateSemaphore = DeviceFuncs->vkCreateSemaphore;
    vkResetFences = DeviceFuncs->vkResetFences;
    vkResetCommandPool = DeviceFuncs->vkResetCommandPool;
    vkBeginCommandBuffer = DeviceFuncs->vkBeginCommandBuffer;
    vkEndCommandBuffer = DeviceFuncs->vkEndCommandBuffer;
    vkQueueSubmit = DeviceFuncs->vkQueueSubmit;
    vkGetFenceStatus = DeviceFuncs->vkGetFenceStatus;
    vkWaitForFences = DeviceFuncs->vkWaitForFences;
    vkCreateRenderPass = DeviceFuncs->vkCreateRenderPass;
    vkCreateImageView = DeviceFuncs->vkCreateImageView;
    vkCreateFramebuffer = DeviceFuncs->vkCreateFramebuffer;
    vkCmdPipelineBarrier = DeviceFuncs->vkCmdPipelineBarrier;
    vkCmdBeginRenderPass = DeviceFuncs->vkCmdBeginRenderPass;
    vkCmdEndRenderPass = DeviceFuncs->vkCmdEndRenderPass;
    vkCreateBuffer = DeviceFuncs->vkCreateBuffer;
    vkGetBufferMemoryRequirements = DeviceFuncs->vkGetBufferMemoryRequirements;
    vkAllocateMemory = DeviceFuncs->vkAllocateMemory;
    vkBindBufferMemory = DeviceFuncs->vkBindBufferMemory;
    vkMapMemory = DeviceFuncs->vkMapMemory;
    vkCmdCopyBuffer = DeviceFuncs->vkCmdCopyBuffer;
    vkCreatePipelineLayout = DeviceFuncs->vkCreatePipelineLayout;
    vkCreateShaderModule = DeviceFuncs->vkCreateShaderModule;
    vkCreateGraphicsPipelines = DeviceFuncs->vkCreateGraphicsPipelines;
    vkCmdSetViewport = DeviceFuncs->vkCmdSetViewport;
    vkCmdSetScissor = DeviceFuncs->vkCmdSetScissor;
    vkCmdBindPipeline = DeviceFuncs->vkCmdBindPipeline;
    vkCmdBindVertexBuffers = DeviceFuncs->vkCmdBindVertexBuffers;
    vkCmdBindIndexBuffer = DeviceFuncs->vkCmdBindIndexBuffer;
    vkCmdDrawIndexed = DeviceFuncs->vkCmdDrawIndexed;

    Assert(DeviceFuncs->SwapchainKHR.Enabled, "VK_KHR_swapchain is not available!");

    vkCreateSwapchainKHR = DeviceFuncs->SwapchainKHR.vkCreateSwapchainKHR;
    vkGetSwapchainImagesKHR = DeviceFuncs->SwapchainKHR.vkGetSwapchainImagesKHR;
    vkAcquireNextImageKHR = DeviceFuncs->SwapchainKHR.vkAcquireNextImageKHR;
    vkQueuePresentKHR = DeviceFuncs->SwapchainKHR.vkQueuePresentKHR;
}