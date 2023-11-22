allocator* allocator::s_DefaultAllocator = nullptr;

allocator* allocator::Get_Default() {
    if(!s_DefaultAllocator) {
        //All allocations will allocate out of the virtual allocator. This is the lowest level OS page allocator that is supported
        //on all operating systems
        static virtual_allocator VirtualAllocator;

        //Default allocator should use the heap allocator as an additional level above the page allocator. 
        static heap MainHeapAllocator(&VirtualAllocator);

        //Make sure its async, this is the default allocator. This can be used anywhere
        static lock_allocator<> MainLockAllocator(&MainHeapAllocator);

        //And track the allocations to check if there are memory leaks or not (tracking is async)
        static tracking_allocator MainTrackingAllocator(&MainLockAllocator);

        s_DefaultAllocator = &MainTrackingAllocator;
    }
    return s_DefaultAllocator;
}

void* allocator::Allocate(size_t Size, allocate_clear ClearFlag) {
    //Default allocator alignment should be twice the pointer size
    return Allocate_Aligned(Size, PTR_SIZE*2, ClearFlag);
}

void* allocator::Allocate_Aligned(size_t Size, size_t Alignment, allocate_clear ClearFlag) {
    Assert(Alignment > 0 && Is_Pow2(Alignment), "Alignment must be a power of 2");

    uptr Offset = Alignment - 1 + sizeof(void*);
    void* P1 = Allocate_Internal(Size+Offset);
    if(!P1) return NULL;

    void** P2 = (void**)(((uptr)(P1) + Offset) & ~(Alignment - 1));
    P2[-1] = P1;

    if(ClearFlag == allocate_clear::Clear) {
        Memory_Clear(P2, Size);
    }
        
    return P2;
}

void allocator::Free(void* Memory) {
    if(Memory) {
        void** P2 = (void**)Memory;
        Free_Internal(P2[-1]);
    }
}

void* operator new(uptr Size) {
    return allocator::Get_Default()->Allocate(Size);
}

void operator delete(void* Memory) noexcept {
    if(Memory) {
        allocator::Get_Default()->Free(Memory);
    }
}

void* operator new[](uptr Size) {
    return allocator::Get_Default()->Allocate(Size);
}

void operator delete[](void* Memory) noexcept {
    if(Memory) {
        allocator::Get_Default()->Free(Memory);
    }
}

void* operator new(uptr Size, allocator* Allocator) noexcept {
    return Allocator->Allocate(Size);
}

void  operator delete(void* Memory, allocator* Allocator) noexcept {
    if(Memory) {
        Allocator->Free(Memory);
    }
}

void* operator new[](uptr Size, allocator* Allocator) noexcept {
    void* Result = Allocator->Allocate(Size);
    return Result;
}

void operator delete[](void* Memory, allocator* Allocator) noexcept {
    if(Memory) {
        Allocator->Free(Memory);
    }
}

#include "virtual_allocator.cpp"
#include "tracking_allocator.cpp"
#include "arena.cpp"
#include "heap.cpp"