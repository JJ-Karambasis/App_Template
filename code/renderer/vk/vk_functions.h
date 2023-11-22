#ifndef VK_FUNCTIONS_H
#define VK_FUNCTIONS_H

#include <vk_loader.h>

//Global functions
static VK_FUNCTION(vkEnumerateInstanceExtensionProperties);
static VK_FUNCTION(vkEnumerateInstanceLayerProperties);
static VK_FUNCTION(vkCreateInstance);

//Instance functions
static VK_FUNCTION(vkEnumeratePhysicalDevices);
static VK_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
static VK_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
static VK_FUNCTION(vkEnumerateDeviceExtensionProperties);
static VK_FUNCTION(vkCreateDevice);
static VK_FUNCTION(vkGetPhysicalDeviceProperties);
static VK_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
static VK_FUNCTION(vkGetDeviceQueue);
static VK_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
static VK_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);

#ifdef DEBUG_BUILD
static VK_FUNCTION(vkCreateDebugUtilsMessengerEXT);
static VK_FUNCTION(vkCreateDebugReportCallbackEXT);
#endif

//Device functions
static VK_FUNCTION(vkCreateCommandPool);
static VK_FUNCTION(vkAllocateCommandBuffers);
static VK_FUNCTION(vkCreateFence);
static VK_FUNCTION(vkCreateSemaphore);
static VK_FUNCTION(vkResetFences);
static VK_FUNCTION(vkResetCommandPool);
static VK_FUNCTION(vkBeginCommandBuffer);
static VK_FUNCTION(vkEndCommandBuffer);
static VK_FUNCTION(vkQueueSubmit);
static VK_FUNCTION(vkGetFenceStatus);
static VK_FUNCTION(vkWaitForFences);
static VK_FUNCTION(vkCreateSwapchainKHR);
static VK_FUNCTION(vkGetSwapchainImagesKHR);
static VK_FUNCTION(vkCreateRenderPass);
static VK_FUNCTION(vkCreateImageView);
static VK_FUNCTION(vkCreateFramebuffer);
static VK_FUNCTION(vkAcquireNextImageKHR);
static VK_FUNCTION(vkCmdPipelineBarrier);
static VK_FUNCTION(vkCmdBeginRenderPass);
static VK_FUNCTION(vkCmdEndRenderPass);
static VK_FUNCTION(vkQueuePresentKHR);
static VK_FUNCTION(vkCreateBuffer);
static VK_FUNCTION(vkGetBufferMemoryRequirements);
static VK_FUNCTION(vkAllocateMemory);
static VK_FUNCTION(vkBindBufferMemory);
static VK_FUNCTION(vkMapMemory);
static VK_FUNCTION(vkCmdCopyBuffer);
static VK_FUNCTION(vkCreatePipelineLayout);
static VK_FUNCTION(vkCreateShaderModule);
static VK_FUNCTION(vkCreateGraphicsPipelines);
static VK_FUNCTION(vkCmdSetViewport);
static VK_FUNCTION(vkCmdSetScissor);
static VK_FUNCTION(vkCmdBindPipeline);
static VK_FUNCTION(vkCmdBindVertexBuffers);
static VK_FUNCTION(vkCmdBindIndexBuffer);
static VK_FUNCTION(vkCmdDrawIndexed);

void VK_Load_Global_Funcs(vk_loader* Loader);
void VK_Load_Instance_Funcs(vk_loader* Loader, const vk_loader_instance_info& InstanceInfo);
void VK_Load_Device_Funcs(vk_loader* Loader, const vk_loader_device_info& DeviceInfo);

#endif