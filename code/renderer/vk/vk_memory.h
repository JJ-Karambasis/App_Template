#ifndef VK_MEMORY_H
#define VK_MEMORY_H

void VK_Set_Allocator();
const VkAllocationCallbacks* VK_Get_Allocator();
s32 VK_Find_Memory_Property_Index(const VkPhysicalDeviceMemoryProperties* MemoryProperties, u32 MemoryTypeBitsRequirement, VkMemoryPropertyFlags RequiredProperties);

#endif