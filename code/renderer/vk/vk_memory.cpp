struct vk_aligned_block {
    uptr ByteSize;
};

void* VKAPI_PTR VK_Alloc(void* UserData, uptr ByteSize, uptr Alignment, VkSystemAllocationScope AllocationScope) {
    Assert(Alignment > 0 && Is_Pow2(Alignment), "Alignment must be a power of 2");

    allocator* Allocator = allocator::Get_Default();
    void* Result = Allocator->Allocate_Aligned(ByteSize, Alignment);

    uptr Offset = Alignment - 1 + sizeof(void*);
    vk_aligned_block* Block = (vk_aligned_block*)Allocator->Allocate(ByteSize+Offset+sizeof(vk_aligned_block));
    if(!Block) return NULL;

    Block->ByteSize = ByteSize;

    void* P1  = Block+1;
    void** P2 = (void**)(((uptr)(P1) + Offset) & ~(Alignment - 1));
    P2[-1] = P1;
        
    return P2;
}

void* VKAPI_PTR VK_Realloc(void* UserData, void* Original, uptr Size, uptr Alignment, VkSystemAllocationScope AllocationScope) {
    if(!Original) return VK_Alloc(UserData, Size, Alignment, AllocationScope);
    Assert(Alignment > 0 && Is_Pow2(Alignment), "Alignment must be a power of 2");
    
    allocator* Allocator = allocator::Get_Default();
    
    void* OriginalUnaligned = ((void**)Original)[-1];
    vk_aligned_block* OriginalBlock = ((vk_aligned_block*)OriginalUnaligned)-1;

    uptr Offset = Alignment - 1 + sizeof(void*);

    vk_aligned_block* NewBlock = (vk_aligned_block*)Allocator->Allocate(Size+Offset+sizeof(vk_aligned_block));
    NewBlock->ByteSize = Size;

    void* P1  = NewBlock+1;
    void** P2 = (void**)(((uptr)(P1) + Offset) & ~(Alignment - 1));
    P2[-1] = P1;

    Memory_Copy(P2, Original, Get_Min(NewBlock->ByteSize, OriginalBlock->ByteSize));

    Allocator->Free(OriginalBlock);

    return P2;
}

void VKAPI_PTR VK_Free(void* UserData, void* Memory) {
    if(Memory) {
        allocator* Allocator = allocator::Get_Default();
        void* OriginalUnaligned = ((void**)Memory)[-1];
        vk_aligned_block* Block = ((vk_aligned_block*)OriginalUnaligned)-1;
        Allocator->Free(Block);
    }
}

static VkAllocationCallbacks G_AllocationCallback;

void VK_Set_Allocator() {
    G_AllocationCallback = {
        .pfnAllocation = VK_Alloc,
        .pfnReallocation = VK_Realloc,
        .pfnFree = VK_Free
    };
}

const VkAllocationCallbacks* VK_Get_Allocator() {
    return &G_AllocationCallback;
}

s32 VK_Find_Memory_Property_Index(const VkPhysicalDeviceMemoryProperties* MemoryProperties, u32 MemoryTypeBitsRequirement, VkMemoryPropertyFlags RequiredProperties) {

    u32 MemoryCount = MemoryProperties->memoryTypeCount;
    for (u32 MemoryIndex = 0; MemoryIndex < MemoryCount; MemoryIndex++) {
        u32 MemoryTypeBits = (1 << MemoryIndex);
        bool IsRequiredMemoryType = MemoryTypeBitsRequirement & MemoryTypeBits;

        const VkMemoryPropertyFlags Properties =
            MemoryProperties->memoryTypes[MemoryIndex].propertyFlags;
        bool HasRequiredProperties = (Properties & RequiredProperties) == RequiredProperties;

        if (IsRequiredMemoryType && HasRequiredProperties)
            return (s32)MemoryIndex;
    }

    // failed to find memory type
    return -1;
}