#ifndef LOCK_ALLOCATOR_H
#define LOCK_ALLOCATOR_H

template <typename lock = mutex>
struct lock_allocator : public allocator {
    allocator* Allocator = nullptr;
    lock       Lock;

    inline lock_allocator() { }
    inline lock_allocator(allocator* _Allocator) : Allocator(_Allocator) { 
        Lock.Init();
    }
    
    inline void* Allocate_Internal(uptr Size) override {
        Lock.Lock();
        void* Result = Allocator->Allocate_Internal(Size);
        Lock.Unlock();
        return Result;
    }
    
    inline void Free_Internal(void* Memory) override{
        if(Memory) {
            Lock.Lock();
            Allocator->Free_Internal(Memory);
            Lock.Unlock();
        }
    }

    inline void Release() {
        Lock.Release();
    }
};


#endif