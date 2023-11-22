#ifndef ALLOCATOR_H
#define ALLOCATOR_H

enum class allocate_clear {
    Clear,
    No_Clear
};

#define DEFAULT_CLEAR_FLAG allocate_clear::Clear

struct allocator {
    template <typename type>
    friend struct lock_allocator;

    friend struct tracking_allocator;

    void* Allocate(size_t Size, allocate_clear ClearFlag = DEFAULT_CLEAR_FLAG);
    void* Allocate_Aligned(size_t Size, size_t Alignment, allocate_clear ClearFlag = DEFAULT_CLEAR_FLAG);
    void  Free(void* Memory);

protected:
    virtual void* Allocate_Internal(size_t Size) = 0;
    virtual void  Free_Internal(void* Memory) = 0;

public:

    static allocator* Get_Default();

    static void Set_Default(allocator* Allocator) {
        s_DefaultAllocator = Allocator;
    } 

    private:
    static allocator* s_DefaultAllocator;
};

void* operator new(uptr Size);
void  operator delete(void* Memory) noexcept;
void* operator new[](uptr Size);
void  operator delete[](void* Memory) noexcept;
void* operator new(uptr Size, allocator* Allocator) noexcept;
void  operator delete(void* Memory, allocator* Allocator) noexcept;
void* operator new[](uptr Size, allocator* Allocator) noexcept;
void  operator delete[](void* Memory, allocator* Allocator) noexcept;

#include "virtual_allocator.h"
#include "tracking_allocator.h"
#include "lock_allocator.h"
#include "arena.h"
#include "heap.h"

#endif