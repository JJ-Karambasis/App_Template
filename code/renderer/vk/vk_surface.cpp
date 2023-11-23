static string VK_Get_Surface_Extension_Name() {
#if   defined(VK_USE_PLATFORM_WIN32_KHR)
    static string SurfaceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    static string SurfaceExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    static string SurfaceExtension(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#else
    #error Not Implemented
#endif

    return SurfaceExtension;
}

static inline bool VK_Is_Surface_Extension(const string& ExtensionName) {
    return ExtensionName == VK_Get_Surface_Extension_Name();
}  

void VK_Set_Surface_Extension(vk_instance_extension_support* InstanceExtensions) {
#if   defined(VK_USE_PLATFORM_WIN32_KHR)
    InstanceExtensions->Win32SurfaceKHR = true;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    InstanceExtensions->AndroidSurfaceKHR = true;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    InstanceExtensions->MetalSurfaceEXT = true;
#else
    #error Not Implemented
#endif
}

bool VK_Has_Surface_Extension(const vk_instance_extension_support& InstanceExtensions) {
#if  defined(VK_USE_PLATFORM_WIN32_KHR)
    return InstanceExtensions.Win32SurfaceKHR;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    return InstanceExtensions.AndroidSurfaceKHR;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    return InstanceExtensions.MetalSurfaceEXT;
#else
    #error Not Implemented
    return false;
#endif
}

bool vk_renderer::Create_Surface(void* WindowData) {
    const vk_instance_funcs* InstanceFuncs = Loader->Load_Instance_Funcs({
        .Instance = Instance,
        .Extensions = InstanceExtensions
    });

#if   defined(VK_USE_PLATFORM_WIN32_KHR)
    win32_window_data* Win32 = (win32_window_data*)WindowData;

    VkWin32SurfaceCreateInfoKHR CreateInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = Win32->Instance,
        .hwnd = Win32->Window
    };

    Assert(InstanceFuncs->Win32SurfaceKHR.Enabled, "Win32 surface is not available!");

    if (InstanceFuncs->Win32SurfaceKHR.vkCreateWin32SurfaceKHR(Instance, &CreateInfo, VK_Get_Allocator(), &Surface) != VK_SUCCESS) {
        return false;
    }

#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    android_window_data* Android = (android_window_data*)WindowData;

    VkAndroidSurfaceCreateInfoKHR CreateInfo = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .window = Android->Window
    };

    Assert(InstanceFuncs->AndroidSurfaceKHR.Enabled, "Android surface is not available!");

    if(InstanceFuncs->AndroidSurfaceKHR.vkCreateAndroidSurfaceKHR(Instance, &CreateInfo, VK_Get_Allocator(), &Surface) != VK_SUCCESS) {
        return false;
    }
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    osx_window_data* OSX = (osx_window_data*)WindowData;
    
    VkMetalSurfaceCreateInfoEXT CreateInfo = {
        .sType  = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
        .pLayer = OSX->Layer
    };
    
    Assert(InstanceFuncs->MetalSurfaceEXT.Enabled, "Metal surface is not available!");
    
    if(InstanceFuncs->MetalSurfaceEXT.vkCreateMetalSurfaceEXT(Instance, &CreateInfo, VK_Get_Allocator(), &Surface) != VK_SUCCESS) {
        return false;
    }
#else
#error Not Implemented
#endif

    return true;
}
