tracking_allocator::tracking_allocator() { }

tracking_allocator::tracking_allocator(allocator* _Allocator) : Allocator(_Allocator) { }

void* tracking_allocator::Allocate_Internal(size_t Size) {
    uptr TotalSize = Size+sizeof(uptr);
    uptr* Result = (uptr*)Allocator->Allocate_Internal(TotalSize);
    if(Result) {
        TotalAllocated += Size;
        CurrentAllocated += Size;
        *Result = Size;
        return Result+1; 
    }

    return nullptr;
}

void tracking_allocator::Free_Internal(void* Memory) {
    if(Memory) {
        uptr* Ptr = ((uptr*)Memory)-1;
        s64 Size = (s64)(*Ptr);
        TotalFreed += Size;
        CurrentAllocated -= Size;
        Allocator->Free_Internal(Ptr);
    }   
}