#if defined(DEBUG_BUILD)
static VkBool32 VKAPI_PTR VK_Debug_Util_Callback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageTypes, 
                                             const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData) {
    Log_Debug(modules::Vulkan, CallbackData->pMessage);
    Invalid_Code();
    return VK_FALSE;
}

static VkBool32 VKAPI_PTR VK_Debug_Report_Callback(VkDebugReportFlagsEXT Flags, VkDebugReportObjectTypeEXT ObjectType, uint64_t Object, size_t Location,
                                                   int32_t MessageCode, const char* LayerPrefix, const char* Message, void* UserData) {
    Log_Debug(modules::Vulkan, Message);
    Invalid_Code();
    return VK_FALSE;
}

bool vk_renderer::Create_Debug_Utils() {
    const vk_instance_funcs* InstanceFuncs = Loader->Load_Instance_Funcs({
        .Instance = Instance,
        .Extensions = InstanceExtensions
    });
    
    if (InstanceFuncs->DebugUtilsEXT.Enabled) {
        VkDebugUtilsMessengerCreateInfoEXT DebugUtilsInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = VK_Debug_Util_Callback
        };

        if (InstanceFuncs->DebugUtilsEXT.vkCreateDebugUtilsMessengerEXT(Instance, &DebugUtilsInfo, VK_Get_Allocator(), &DebugMessenger) != VK_SUCCESS) {
            return false;
        }
    }
    else if (InstanceFuncs->DebugReportEXT.Enabled) {
        VkDebugReportCallbackCreateInfoEXT DebugReportInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
            .flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT,
            .pfnCallback = VK_Debug_Report_Callback
        };

        if (InstanceFuncs->DebugReportEXT.vkCreateDebugReportCallbackEXT(Instance, &DebugReportInfo, VK_Get_Allocator(), &DebugReportCallback) != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

#endif
