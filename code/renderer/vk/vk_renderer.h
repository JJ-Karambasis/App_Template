#ifndef VK_RENDERER_H
#define VK_RENDERER_H

#include <app.h>

#if defined(OS_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#if defined(OS_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

#if defined(OS_OSX)
#define VK_USE_PLATFORM_METAL_EXT

# if !defined(OS_IOS)
# define VK_ENABLE_BETA_EXTENSIONS
# endif

#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include "vk_functions.h"
#include "vk_memory.h"

struct vk_upload_buffer {
    VkBuffer       Buffer;
    VkDeviceMemory Memory;
    u8*            Ptr;
    VkDeviceSize   Size;
    VkDeviceSize   Used;

    VkDeviceSize Upload(const const_buffer& Data);
    void Clear();
};

struct vk_buffer_arena {
    VkBuffer       Buffer;
    VkDeviceMemory Memory;
    VkDeviceSize   Size;
    VkDeviceSize   Used;
    VkDeviceSize   Push(VkDeviceSize PushSize);
};


struct vk_frame_context {
    VkCommandPool    CmdPool;
    VkCommandBuffer  CmdBuffer;
    VkFence          Fence;
    VkSemaphore      AcquireLock;
    VkSemaphore      ExecuteLock;
    vk_upload_buffer UploadBuffer;
};

struct vk_device {
    VkPhysicalDevice                 GPU;
    VkPhysicalDeviceProperties       Properties;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    u32                              GraphicsQueueFamilyIndex;
    u32                              PresentQueueFamilyIndex;
    VkDevice                         Device;
    vk_device_extension_support      Extensions;
    VkQueue                          GraphicsQueue;
    VkQueue                          PresentQueue;

    inline VkDevice Get() {
        return Device;
    }
};

struct vk_swapchain {
    VkSwapchainKHR Swapchain;
    VkFormat       Format;
    array<VkImage> Images;
    VkExtent2D     Extent;

    inline VkSwapchainKHR& Get() {
        return Swapchain;
    }
};

struct vk_renderer : public app_renderer {
    arena      Arena;
    vk_loader* Loader;
    VkInstance Instance;
    vk_instance_extension_support InstanceExtensions;
    VkSurfaceKHR Surface;
    vk_device    Device;
    vk_swapchain Swapchain;
    u32 CurrentFrameIndex;
    array<vk_frame_context> Frames;
    vk_frame_context* Frame;

    array<VkImageView>   SwapchainImageViews;
    array<VkFramebuffer> SwapchainFramebuffers;
    vk_buffer_arena      VertexBufferArena;
    vk_buffer_arena      IndexBufferArena;
    VkRenderPass         RenderPass;
    VkPipelineLayout     PipelineLayout;
    VkPipeline           Pipeline;

    bool Create_Instance();
    bool Create_Surface(void* WindowData);
    bool Create_Device();
    bool Create_Swapchain();
    bool Create_Frames(u32 FrameCount);
    bool Build_Initial_Resources();
    
#ifdef DEBUG_BUILD
    VkDebugUtilsMessengerEXT DebugMessenger;
    VkDebugReportCallbackEXT DebugReportCallback;
    bool Create_Debug_Utils();
#endif
};

#endif