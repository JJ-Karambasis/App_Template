#ifndef TRACKING_ALLOCATOR_H
#define TRACKING_ALLOCATOR_H

struct tracking_allocator : public allocator {
    allocator* Allocator = nullptr;

    std::atomic_uint64_t TotalAllocated;
    std::atomic_uint64_t TotalFreed;
    std::atomic_uint64_t CurrentAllocated;

    tracking_allocator();
    tracking_allocator(allocator* _Allocator);

protected:
    void* Allocate_Internal(size_t Size) override;
    void  Free_Internal(void* Memory) override;
};

#endif