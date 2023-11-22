#if defined(OS_WIN32)

void* virtual_allocator::Reserve(uptr Size) {
    return VirtualAlloc(NULL, Size, MEM_RESERVE, PAGE_READWRITE);
}

void* virtual_allocator::Commit(void* Memory, uptr Size) {
    return VirtualAlloc(Memory, Size, MEM_COMMIT, PAGE_READWRITE);
}

void virtual_allocator::Decommit(void* Memory, uptr Size) {
    VirtualFree(Memory, Size, MEM_DECOMMIT);
}

void  virtual_allocator::Release(void* Memory, uptr Size) {
    VirtualFree(Memory, 0, MEM_RELEASE);
}

#elif defined(OS_POSIX)

void* virtual_allocator::Reserve(uptr Size) {
    void* Result = mmap(nullptr, Size, PROT_NONE, (MAP_PRIVATE|MAP_ANONYMOUS), -1, 0);
    if(Result) {
        msync(Result, Size, (MS_SYNC|MS_INVALIDATE));
    }
    return Result;
}

void* virtual_allocator::Commit(void* Memory, uptr Size) {
    if(Memory) {
        void* Result = mmap(Memory, Size, (PROT_READ|PROT_WRITE), (MAP_FIXED|MAP_SHARED|MAP_ANONYMOUS), -1, 0);
        msync(Memory, Size, (MS_SYNC|MS_INVALIDATE));
        return Result;
    }
    return nullptr;
}

void virtual_allocator::Decommit(void* Memory, uptr Size) {
    if(Memory) {
        mmap(Memory, Size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        msync(Memory, Size, MS_SYNC|MS_INVALIDATE);
    }
}

void virtual_allocator::Release(void* Memory, uptr Size) {
    if(Memory) {
        msync(Memory, Size, MS_SYNC);
        munmap(Memory, Size);
    }
}

#else
# error Not Implemented
#endif

inline void* virtual_allocator::Allocate_Internal(uptr Size) {
    Size += sizeof(uptr);
    uptr* Result = (uptr*)Reserve(Size);
    if(Result) {
        if(Commit(Result, Size)) {
            *Result = Size;
            return Result+1;
        } else {
            Release(Result, Size);
        }
    }

    return nullptr;
}

inline void virtual_allocator::Free_Internal(void* Memory) {
    if(Memory) {
        uptr* Ptr = ((uptr*)Memory)-1;
        uptr Size = *Ptr;
        Decommit(Ptr, Size);
        Release(Ptr, Size);
    }
}