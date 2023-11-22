#include "vk_renderer.h"

#include "vk_debug.cpp"
#include "vk_surface.cpp"

static vk_upload_buffer VK_Create_Upload_Buffer(vk_device* Device, VkDeviceSize Size) {
    
    VkBufferCreateInfo BufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = Size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkBuffer Buffer;
    if(vkCreateBuffer(Device->Get(), &BufferInfo, VK_Get_Allocator(), &Buffer) != VK_SUCCESS) {
        Log_Error(modules::Vulkan, "Failed to create the vulkan upload buffer!");
        return {};
    }

    VkMemoryRequirements MemoryRequirements = {};
    vkGetBufferMemoryRequirements(Device->Get(), Buffer, &MemoryRequirements);

    s32 MemoryIndex = VK_Find_Memory_Property_Index(&Device->MemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if(MemoryIndex == -1) {
        Log_Error(modules::Vulkan, "Failed to find a valid vulkan memory type!");
        return {};
    }

    VkMemoryAllocateInfo AllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = MemoryRequirements.size,
        .memoryTypeIndex = (u32)MemoryIndex
    };

    VkDeviceMemory Memory;
    if(vkAllocateMemory(Device->Get(), &AllocateInfo, VK_Get_Allocator(), &Memory) != VK_SUCCESS) {
        Log_Error(modules::Vulkan, "Failed to allocate vulkan memory!");
        return {};
    }

    if(vkBindBufferMemory(Device->Get(), Buffer, Memory, 0) != VK_SUCCESS) {
        Log_Error(modules::Vulkan, "Failed to bind vulkan memory to upload buffer!");
        return {};
    }

    u8* Ptr;
    if(vkMapMemory(Device->Get(), Memory, 0, MemoryRequirements.size, 0, (void**)&Ptr) != VK_SUCCESS) {
        Log_Error(modules::Vulkan, "Failed to map vulkan memory to upload buffer!");
        return {};
    }

    return {
        .Buffer = Buffer,
        .Memory = Memory,
        .Ptr = Ptr,
        .Size = MemoryRequirements.size,
    };
}

VkDeviceSize vk_upload_buffer::Upload(const const_buffer& Data) {
    Assert(Used+Data.Size <= Size, "Overflow!");
    VkDeviceSize Result = Used;
    Memory_Copy(Ptr+Used, Data.Ptr, Data.Size);
    Used += Data.Size;
    return Result;
}

void vk_upload_buffer::Clear() {
    Used = 0;
}

static vk_buffer_arena VK_Create_Buffer_Arena(vk_device* Device, size_t Size, VkBufferUsageFlags UsageFlags) {
    
    VkBufferCreateInfo BufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = Size,
        .usage = UsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkBuffer Buffer;
    if(vkCreateBuffer(Device->Get(), &BufferInfo, VK_Get_Allocator(), &Buffer) != VK_SUCCESS) {
        Log_Error(modules::Vulkan, "Failed to create the vulkan arena buffer!");
        return {};
    }

    VkMemoryRequirements MemoryRequirements = {};
    vkGetBufferMemoryRequirements(Device->Get(), Buffer, &MemoryRequirements);

    s32 MemoryIndex = VK_Find_Memory_Property_Index(&Device->MemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if(MemoryIndex == -1) {
        Log_Error(modules::Vulkan, "Failed to find a valid vulkan memory type!");
        return {};
    }

    VkMemoryAllocateInfo AllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = MemoryRequirements.size,
        .memoryTypeIndex = (u32)MemoryIndex
    };

    VkDeviceMemory Memory;
    if(vkAllocateMemory(Device->Get(), &AllocateInfo, VK_Get_Allocator(), &Memory) != VK_SUCCESS) {
        Log_Error(modules::Vulkan, "Failed to allocate vulkan memory!");
        return {};
    }

    if(vkBindBufferMemory(Device->Get(), Buffer, Memory, 0) != VK_SUCCESS) {
        Log_Error(modules::Vulkan, "Failed to bind vulkan memory to arena buffer!");
        return {};
    }

    return {
        .Buffer = Buffer,
        .Memory = Memory,
        .Size = MemoryRequirements.size,
    };
}


VkDeviceSize vk_buffer_arena::Push(VkDeviceSize PushSize) {
    Assert(Used+PushSize <= Size, "Overflow!");
    VkDeviceSize Result = Used;
    Used += PushSize;
    return Result;
}

bool vk_renderer::Create_Frames(u32 FrameCount) {
    Frames = array<vk_frame_context>(&Arena, FrameCount);
    for(vk_frame_context& FrameContext : Frames) {
        VkCommandPoolCreateInfo CmdPoolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = Device.GraphicsQueueFamilyIndex,
        };

        if (vkCreateCommandPool(Device.Get(), &CmdPoolInfo, VK_Get_Allocator(), &FrameContext.CmdPool) != VK_SUCCESS) {
            return false;
        }

        VkCommandBufferAllocateInfo CmdBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = FrameContext.CmdPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        if (vkAllocateCommandBuffers(Device.Get(), &CmdBufferAllocateInfo, &FrameContext.CmdBuffer) != VK_SUCCESS) {
            return false;
        }

        VkFenceCreateInfo FenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        if (vkCreateFence(Device.Get(), &FenceInfo, VK_Get_Allocator(), &FrameContext.Fence) != VK_SUCCESS) {
            return false;
        }

        VkSemaphoreCreateInfo SemaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        
        if (vkCreateSemaphore(Device.Get(), &SemaphoreInfo, VK_Get_Allocator(), &FrameContext.AcquireLock) != VK_SUCCESS) {
            return false;
        }

        if (vkCreateSemaphore(Device.Get(), &SemaphoreInfo, VK_Get_Allocator(), &FrameContext.ExecuteLock) != VK_SUCCESS) {
            return false;
        }

        FrameContext.UploadBuffer = VK_Create_Upload_Buffer(&Device, MB(8));
        if (!FrameContext.UploadBuffer.Buffer) {
            return false;
        }
    }

    CurrentFrameIndex = 0;
    Frame = &Frames[CurrentFrameIndex];

    vkResetFences(Device.Get(), 1, &Frame->Fence);
    vkResetCommandPool(Device.Get(), Frame->CmdPool, 0);

    VkCommandBufferBeginInfo StartCmdBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if (vkBeginCommandBuffer(Frame->CmdBuffer, &StartCmdBufferBeginInfo) != VK_SUCCESS) {
        return false;
    }

    return true;
}


APP_RENDERER_RENDER(App_Renderer_Render) {
    vk_renderer* Renderer = (vk_renderer*)_Renderer;

    vk_device* Device = &Renderer->Device;
    vk_swapchain* Swapchain = &Renderer->Swapchain;

    vk_frame_context* Frame = Renderer->Frame;

    u32 ImageIndex;
    VkResult SwapchainStatus = vkAcquireNextImageKHR(Device->Get(), Swapchain->Get(), UINT64_MAX, Frame->AcquireLock, VK_NULL_HANDLE, &ImageIndex);
    if (SwapchainStatus == VK_ERROR_OUT_OF_DATE_KHR || SwapchainStatus == VK_SUBOPTIMAL_KHR) {
        //TODO: Recreate swapchain
        Not_Implemented();
    }

    VkClearValue ClearValue = {
        .color = {
            .float32 = {0, 0, 1, 0}
        }
    };

    VkRenderPassBeginInfo RenderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = Renderer->RenderPass,
        .framebuffer = Renderer->SwapchainFramebuffers[ImageIndex],
        .renderArea = {{}, Swapchain->Extent},
        .clearValueCount = 1,
        .pClearValues = &ClearValue
    };

    VkImageMemoryBarrier PreRenderBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image = Swapchain->Images[ImageIndex],
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    };

    vkCmdPipelineBarrier(Frame->CmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_DEPENDENCY_BY_REGION_BIT, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &PreRenderBarrier);

    vkCmdBeginRenderPass(Frame->CmdBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport Viewport = {
        .width = (f32)Swapchain->Extent.width,
        .height = (f32)Swapchain->Extent.height,
        .maxDepth = 1.0f
    };

    VkRect2D Scissor = { {}, Swapchain->Extent };
    vkCmdSetViewport(Frame->CmdBuffer, 0, 1, &Viewport);
    vkCmdSetScissor(Frame->CmdBuffer, 0, 1, &Scissor);

    vkCmdBindPipeline(Frame->CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Renderer->Pipeline);

    VkDeviceSize VertexBufferOffset = 0;
    vkCmdBindVertexBuffers(Frame->CmdBuffer, 0, 1, &Renderer->VertexBufferArena.Buffer, &VertexBufferOffset);
    vkCmdBindIndexBuffer(Frame->CmdBuffer, Renderer->IndexBufferArena.Buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(Frame->CmdBuffer, 3, 1, 0, 0, 0);

    vkCmdEndRenderPass(Frame->CmdBuffer);

    VkImageMemoryBarrier PostRenderBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = Swapchain->Images[ImageIndex],
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    };

    vkCmdPipelineBarrier(Frame->CmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_DEPENDENCY_BY_REGION_BIT, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &PostRenderBarrier);

    vkEndCommandBuffer(Frame->CmdBuffer);

    VkPipelineStageFlags WaitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo SubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &Frame->AcquireLock,
        .pWaitDstStageMask = &WaitStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &Frame->CmdBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &Frame->ExecuteLock
    };

    vkQueueSubmit(Device->GraphicsQueue, 1, &SubmitInfo, Frame->Fence);

    VkPresentInfoKHR PresentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &Frame->ExecuteLock,
        .swapchainCount = 1,
        .pSwapchains = &Swapchain->Get(),
        .pImageIndices = &ImageIndex
    };

    vkQueuePresentKHR(Device->PresentQueue, &PresentInfo);

    Renderer->CurrentFrameIndex = (Renderer->CurrentFrameIndex + 1) % Safe_U32(Renderer->Frames.Count);
    vk_frame_context* NextFrame = &Renderer->Frames[Renderer->CurrentFrameIndex];

    if (vkGetFenceStatus(Device->Get(), NextFrame->Fence) == VK_NOT_READY) {
        vkWaitForFences(Device->Get(), 1, &NextFrame->Fence, VK_TRUE, UINT64_MAX);
    }
    vkResetFences(Device->Get(), 1, &NextFrame->Fence);
    vkResetCommandPool(Device->Get(), NextFrame->CmdPool, 0);
    NextFrame->UploadBuffer.Clear();

    VkCommandBufferBeginInfo CmdBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(NextFrame->CmdBuffer, &CmdBufferBeginInfo);

    Renderer->Frame = NextFrame;
}

extern "C" shared_function APP_RENDERER_RELOAD(App_Renderer_Reload) {
    vk_renderer* Renderer = (vk_renderer*)_Renderer;

    core::Set(Core);
    Renderer->Render = App_Renderer_Render;

    VK_Set_Allocator();

    VK_Load_Global_Funcs(Renderer->Loader);
    if(Renderer->Instance) {
        VK_Load_Instance_Funcs(Renderer->Loader, {
            .Instance = Renderer->Instance,
            .Extensions = Renderer->InstanceExtensions
        });
    }

    if(Renderer->Device.Get()) {
        VK_Load_Device_Funcs(Renderer->Loader, {
            .Instance = Renderer->Instance,
            .Device = Renderer->Device.Get(),
            .Extensions = Renderer->Device.Extensions
        });
    }
}

extern "C" shared_function APP_RENDERER_CREATE(App_Renderer_Create) {
    arena Arena(Core->Allocator);
    vk_renderer* Renderer = new(&Arena) vk_renderer;
    
    Renderer->Arena  = Arena;
    Renderer->Loader = VK_Get_Loader();

    App_Renderer_Reload(Renderer, Core);    

    if (!Renderer->Create_Instance()) {
        Log_Error(modules::Vulkan, "Failed to create the vulkan instance!");

    }

#ifdef DEBUG_BUILD
    if (!Renderer->Create_Debug_Utils()) {
        Log_Error(modules::Vulkan, "Failed to create the vulkan debug utils!");
        return nullptr;
    }
#endif

    if (!Renderer->Create_Surface(WindowData)) {
        Log_Error(modules::Vulkan, "Failed to create the vulkan surface!");
        return nullptr;
    }

    if (!Renderer->Create_Device()) {
        Log_Error(modules::Vulkan, "Failed to create the vulkan device!");
        return nullptr;
    }

    if (!Renderer->Create_Swapchain()) {
        Log_Error(modules::Vulkan, "Failed to create the vulkan device!");
        return nullptr;
    }

    if (!Renderer->Create_Frames(2)) {
        return nullptr;
    }

    if (!Renderer->Build_Initial_Resources()) {
        return nullptr;
    }

    return Renderer;
}

bool vk_renderer::Create_Instance() {
    scratch Scratch = Get_Scratch();

    u32 PropertyCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, VK_NULL_HANDLE);

    array<VkExtensionProperties> ExtensionProps(&Scratch, PropertyCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, ExtensionProps.Ptr);

    vector<const char*> Extensions(&Scratch);

    for (const VkExtensionProperties& ExtensionProp : ExtensionProps) {
        string ExtensionName(ExtensionProp.extensionName);
        Log_Debug(modules::Vulkan, ExtensionName.Str);

        if (ExtensionName == string(VK_KHR_SURFACE_EXTENSION_NAME)) {
            InstanceExtensions.SurfaceKHR = true;
            Extensions.Push(VK_KHR_SURFACE_EXTENSION_NAME);
        }
        else if (ExtensionName == string(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            InstanceExtensions.GetPhysicalDeviceProperties2KHR = true;
            Extensions.Push(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }
        else if (VK_Is_Surface_Extension(ExtensionName)) {
            VK_Set_Surface_Extension(&InstanceExtensions);
            Extensions.Push(ExtensionName.Str);
        }
#if defined(DEBUG_BUILD)
        else if (ExtensionName == string(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            InstanceExtensions.DebugUtilsEXT = true;
            Extensions.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else if (ExtensionName == string(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
            InstanceExtensions.DebugReportEXT = true;
            Extensions.Push(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
#endif
    }

    if (!InstanceExtensions.SurfaceKHR) {
        Log_Error(modules::Vulkan, "Failed to find the required vulkan instance extension '%s'", VK_KHR_SURFACE_EXTENSION_NAME);
        return false;
    }

    if (!VK_Has_Surface_Extension(InstanceExtensions)) {
        Log_Error(modules::Vulkan, "Failed to find the required vulkan instance extension '%s'", VK_Get_Surface_Extension_Name().Str);
        return false;
    }

#if defined(VK_ENABLE_BETA_EXTENSIONS)
    Extensions.Push(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    //No layers for release builds. Debug builds will require std validation
    vector<const char*> Layers(&Scratch);

    //Validation layers on android don't work for some reason. At least on armv7
#if defined(DEBUG_BUILD)
    u32 LayerCount;
    vkEnumerateInstanceLayerProperties(&LayerCount, VK_NULL_HANDLE);

    array<VkLayerProperties> LayerProps(&Scratch, LayerCount);
    vkEnumerateInstanceLayerProperties(&LayerCount, LayerProps.Ptr);

    for (const VkLayerProperties& LayerProp : LayerProps) {
        string LayerName(LayerProp.layerName);
        if (LayerName == String_Lit("VK_LAYER_KHRONOS_validation")) {
            Layers.Push(LayerProp.layerName);
        }
    }
#endif

    VkApplicationInfo ApplicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "App",
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = "App_Engine",
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo InstanceInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &ApplicationInfo,
        .enabledLayerCount = Safe_U32(Layers.Count),
        .ppEnabledLayerNames = Layers.Ptr,
        .enabledExtensionCount = Safe_U32(Extensions.Count),
        .ppEnabledExtensionNames = Extensions.Ptr
    };

#ifdef VK_ENABLE_BETA_EXTENSIONS
    InstanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkResult Status = vkCreateInstance(&InstanceInfo, VK_Get_Allocator(), &Instance);
    if (Status != VK_SUCCESS) {
        Log_Error(modules::Vulkan, "Failed to create the vulkan instance!");
        return false;
    }

    VK_Load_Instance_Funcs(Loader, {
        .Instance = Instance,
        .Extensions = InstanceExtensions
    });

    return true;
}

bool vk_renderer::Create_Device() {
    scratch Scratch = Get_Scratch();

    u32 PhysicalDeviceCount;
    vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, VK_NULL_HANDLE);

    array<VkPhysicalDevice> PhysicalDevices(Scratch.Get_Arena(), PhysicalDeviceCount);
    vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.Ptr);

    for (VkPhysicalDevice PhysicalDevice : PhysicalDevices) {


        u32 QueueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, VK_NULL_HANDLE);

        array<VkQueueFamilyProperties> QueueFamilies(&Scratch, QueueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilies.Ptr);

        u32 GraphicsQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        u32 PresentQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        for (uptr QueueFamilyIndex : Indices(QueueFamilies)) {
            if (QueueFamilies[QueueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                GraphicsQueueFamilyIndex = Safe_U32(QueueFamilyIndex);
                break;
            }
        }

        if (GraphicsQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED) {

            VkBool32 IsSupported;
            vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, GraphicsQueueFamilyIndex, Surface, &IsSupported);

            if (IsSupported) {
                PresentQueueFamilyIndex = GraphicsQueueFamilyIndex;
            }
            else {
                for (uptr QueueFamilyIndex : Indices(QueueFamilies)) {
                    vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, Safe_U32(QueueFamilyIndex), Surface, &IsSupported);
                    if (IsSupported) {
                        PresentQueueFamilyIndex = Safe_U32(QueueFamilyIndex);
                        break;
                    }
                }
            }

            if (PresentQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED) {

                f32 Priority = 1.0f;

                u32 DeviceQueueCreateCount = 1;
                VkDeviceQueueCreateInfo DeviceQueueCreateInfos[2] = {
                    {
                        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                        .queueFamilyIndex = GraphicsQueueFamilyIndex,
                        .queueCount = 1,
                        .pQueuePriorities = &Priority
                    }
                };

                if (GraphicsQueueFamilyIndex != PresentQueueFamilyIndex) {
                    DeviceQueueCreateCount = 2;

                    DeviceQueueCreateInfos[1] = {
                        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                        .queueFamilyIndex = PresentQueueFamilyIndex,
                        .queueCount = 1,
                        .pQueuePriorities = &Priority
                    };
                }

                vk_device_extension_support DeviceExtensions = {
                    .SwapchainKHR = true
                };

                vector<const char*> Extensions(&Scratch);
                Extensions.Push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#ifdef VK_ENABLE_BETA_EXTENSIONS
                Extensions.Push(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
                VkDeviceCreateInfo DeviceCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .queueCreateInfoCount = DeviceQueueCreateCount,
                    .pQueueCreateInfos = DeviceQueueCreateInfos,
                    .enabledExtensionCount = Safe_U32(Extensions.Count),
                    .ppEnabledExtensionNames = Extensions.Ptr
                };

                VkDevice DeviceHandle;
                if (vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, VK_Get_Allocator(), &DeviceHandle) == VK_SUCCESS) {
                    VkPhysicalDeviceProperties Properties;
                    VkPhysicalDeviceMemoryProperties MemoryProperties;

                    vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);
                    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemoryProperties);

                    VkQueue GraphicsQueue, PresentQueue;
                    vkGetDeviceQueue(DeviceHandle, GraphicsQueueFamilyIndex, 0, &GraphicsQueue);

                    if (GraphicsQueueFamilyIndex == PresentQueueFamilyIndex) {
                        PresentQueue = GraphicsQueue;
                    }
                    else {
                        vkGetDeviceQueue(DeviceHandle, PresentQueueFamilyIndex, 0, &PresentQueue);
                    }

                    Device = {
                        .GPU = PhysicalDevice,
                        .Properties = Properties,
                        .MemoryProperties = MemoryProperties,
                        .GraphicsQueueFamilyIndex = GraphicsQueueFamilyIndex,
                        .PresentQueueFamilyIndex = PresentQueueFamilyIndex,
                        .Device = DeviceHandle,
                        .Extensions = DeviceExtensions,
                        .GraphicsQueue = GraphicsQueue,
                        .PresentQueue = PresentQueue
                    };


                    VK_Load_Device_Funcs(Loader, {
                        .Instance = Instance,
                        .Device = Device.Get(),
                        .Extensions = Device.Extensions
                    });

                    return true;
                }
            }
        }
    }

    Log_Error(modules::Vulkan, "Could not find a valid gpu that supported vulkan graphics!");
    return false;
}

bool vk_renderer::Create_Swapchain() {
    scratch Scratch = Get_Scratch();

    VkSurfaceCapabilitiesKHR SurfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device.GPU, Surface, &SurfaceCaps);

    VkCompositeAlphaFlagBitsKHR CompositeAlphaFlags = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    if (SurfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
        CompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    else if (SurfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
        CompositeAlphaFlags = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
    else {
        Log_Error(modules::Vulkan, "Invalid vulkan composite alpha properties for the swapchain!");
        return false;
    }

    //Max of 3 swapchain images
    u32 MinImageCount = Get_Min(SurfaceCaps.maxImageCount, 3);

    if (MinImageCount < 2) {
        Log_Error(modules::Vulkan, "Not enough images for the vulkan swapchain!");
        return false;
    }

    VkFormat TargetFormat = VK_FORMAT_R8G8B8A8_UNORM;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    TargetFormat = VK_FORMAT_B8G8R8A8_UNORM;
#endif

    u32 FormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device.GPU, Surface, &FormatCount, VK_NULL_HANDLE);

    array<VkSurfaceFormatKHR> Formats(&Scratch, FormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device.GPU, Surface, &FormatCount, Formats.Ptr);

    array_find Find = Formats.Find_First([TargetFormat](auto Format) {
        return Format.format == TargetFormat;
    });

    VkSurfaceFormatKHR SurfaceFormat = Find.Found ? Formats[Find.Index] : Formats[0];

    VkSwapchainCreateInfoKHR SwapchainInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = Surface,
        .minImageCount = MinImageCount,
        .imageFormat = SurfaceFormat.format,
        .imageColorSpace = SurfaceFormat.colorSpace,
        .imageExtent = SurfaceCaps.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = CompositeAlphaFlags,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };

    if (Device.PresentQueueFamilyIndex != Device.GraphicsQueueFamilyIndex) {
        SwapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        u32* QueueFamilyIndices = Scratch.Push_Array<u32>(2);
        QueueFamilyIndices[0] = Device.GraphicsQueueFamilyIndex;
        QueueFamilyIndices[1] = Device.PresentQueueFamilyIndex;

        SwapchainInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }

    VkSwapchainKHR SwapchainHandle;
    if (vkCreateSwapchainKHR(Device.Get(), &SwapchainInfo, VK_Get_Allocator(), &SwapchainHandle) != VK_SUCCESS) {
        Log_Error(modules::Vulkan, "Failed to create the vulkan swapchain!");
        return false;
    }

    u32 ImageCount;
    vkGetSwapchainImagesKHR(Device.Get(), SwapchainHandle, &ImageCount, VK_NULL_HANDLE);

    array<VkImage> Images(&Arena, ImageCount);
    vkGetSwapchainImagesKHR(Device.Get(), SwapchainHandle, &ImageCount, Images.Ptr);

    Swapchain = {
        .Swapchain = SwapchainHandle,
        .Format = SurfaceFormat.format,
        .Images = Images,
        .Extent = SurfaceCaps.currentExtent
    };

    return true;
}

bool vk_renderer::Build_Initial_Resources() {
    VertexBufferArena = VK_Create_Buffer_Arena(&Device, MB(32), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (!VertexBufferArena.Buffer) {
        return false;
    }

    IndexBufferArena = VK_Create_Buffer_Arena(&Device, MB(32), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    if (!IndexBufferArena.Buffer) {
        return false;
    }

    SwapchainImageViews = array<VkImageView>(&Arena, Swapchain.Images.Count);
    SwapchainFramebuffers = array<VkFramebuffer>(&Arena, Swapchain.Images.Count);

    VkAttachmentDescription Attachment = {
        .format = Swapchain.Format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference AttachmentRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    VkSubpassDescription Subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &AttachmentRef
    };

    VkRenderPassCreateInfo RenderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &Attachment,
        .subpassCount = 1,
        .pSubpasses = &Subpass
    };

    if (vkCreateRenderPass(Device.Get(), &RenderPassInfo, VK_Get_Allocator(), &RenderPass) != VK_SUCCESS) {
        return false;
    }

    for (uptr i : Indices(Swapchain.Images)) {
        VkImageViewCreateInfo ImageViewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = Swapchain.Images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = Swapchain.Format,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
        };

        if (vkCreateImageView(Device.Get(), &ImageViewInfo, VK_Get_Allocator(), &SwapchainImageViews[i]) != VK_SUCCESS) {
            return false;
        }

        VkFramebufferCreateInfo FramebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = RenderPass,
            .attachmentCount = 1,
            .pAttachments = &SwapchainImageViews[i],
            .width = Swapchain.Extent.width,
            .height = Swapchain.Extent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(Device.Get(), &FramebufferInfo, VK_Get_Allocator(), &SwapchainFramebuffers[i]) != VK_SUCCESS) {
            return false;
        }
    }

    vec3 TriangleVertices[] = {
        vec3(-0.5f, 0.5f, 0.0f),
        vec3(0.5f, 0.5f, 0.0f),
        vec3(0.0f, -0.5f, 0.0f)
    };

    u16 TriangleIndices[] = {
        0, 1, 2
    };

    VkDeviceSize DstVertexOffset = VertexBufferArena.Push(sizeof(TriangleVertices));
    VkDeviceSize DstIndexOffset = IndexBufferArena.Push(sizeof(TriangleIndices));
    VkDeviceSize SrcVertexOffset = Frame->UploadBuffer.Upload(TriangleVertices);
    VkDeviceSize SrcIndexOffset = Frame->UploadBuffer.Upload(TriangleIndices);

    VkBufferCopy VertexBufferCopy = { SrcVertexOffset, DstVertexOffset, sizeof(TriangleVertices) };
    VkBufferCopy IndexBufferCopy = { SrcIndexOffset, DstIndexOffset, sizeof(TriangleIndices) };

    vkCmdCopyBuffer(Frame->CmdBuffer, Frame->UploadBuffer.Buffer, VertexBufferArena.Buffer, 1, &VertexBufferCopy);
    vkCmdCopyBuffer(Frame->CmdBuffer, Frame->UploadBuffer.Buffer, IndexBufferArena.Buffer, 1, &IndexBufferCopy);

    VkPipelineLayoutCreateInfo PipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };

    if (vkCreatePipelineLayout(Device.Get(), &PipelineLayoutInfo, VK_Get_Allocator(), &PipelineLayout) != VK_SUCCESS) {
        return false;
    }

    scratch Scratch = Get_Scratch();
    const_buffer VSShader = Read_Entire_File(&Scratch, String_Lit("shaders/app_vs.shader"));
    const_buffer PSShader = Read_Entire_File(&Scratch, String_Lit("shaders/app_ps.shader"));

    VkShaderModuleCreateInfo VSShaderModuleInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = VSShader.Size,
        .pCode = (const u32*)VSShader.Ptr
    };

    VkShaderModuleCreateInfo PSShaderModuleInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = PSShader.Size,
        .pCode = (const u32*)PSShader.Ptr
    };

    VkShaderModule VSShaderModule, PSShaderModule;
    if (vkCreateShaderModule(Device.Get(), &VSShaderModuleInfo, VK_Get_Allocator(), &VSShaderModule) != VK_SUCCESS) {
        return false;
    }

    if (vkCreateShaderModule(Device.Get(), &PSShaderModuleInfo, VK_Get_Allocator(), &PSShaderModule) != VK_SUCCESS) {
        return false;
    }

    VkPipelineShaderStageCreateInfo Stages[] = { {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = VSShaderModule,
            .pName = "VS_Main"
        }, {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = PSShaderModule,
            .pName = "PS_Main"
        }
    };

    VkVertexInputBindingDescription VertexBindings = {
        .binding = 0,
        .stride = sizeof(vec3),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription VertexAttribs = {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0
    };

    VkPipelineVertexInputStateCreateInfo VertexInputState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &VertexBindings,
        .vertexAttributeDescriptionCount = 1,
        .pVertexAttributeDescriptions = &VertexAttribs
    };

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    VkPipelineViewportStateCreateInfo ViewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    VkPipelineRasterizationStateCreateInfo RasterizationState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo MultisampleState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineDepthStencilStateCreateInfo DepthStencilState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState ColorAttachmentState = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo ColorBlendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &ColorAttachmentState
    };

    VkDynamicState DynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo DynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = Array_Count(DynamicStates),
        .pDynamicStates = DynamicStates
    };

    VkGraphicsPipelineCreateInfo GraphicsPipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = Array_Count(Stages),
        .pStages = Stages,
        .pVertexInputState = &VertexInputState,
        .pInputAssemblyState = &InputAssemblyState,
        .pViewportState = &ViewportState,
        .pRasterizationState = &RasterizationState,
        .pMultisampleState = &MultisampleState,
        .pDepthStencilState = &DepthStencilState,
        .pColorBlendState = &ColorBlendState,
        .pDynamicState = &DynamicState,
        .layout = PipelineLayout,
        .renderPass = RenderPass,
        .subpass = 0
    };

    if (vkCreateGraphicsPipelines(Device.Get(), VK_NULL_HANDLE, 1, &GraphicsPipelineInfo, VK_Get_Allocator(), &Pipeline) != VK_SUCCESS) {
        return false;
    }

    return true;
}

#include "vk_functions.cpp"
#include "vk_memory.cpp"
#include <core.cpp>