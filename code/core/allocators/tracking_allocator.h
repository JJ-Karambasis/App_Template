#ifndef TRACKING_ALLOCATOR_H
#define TRACKING_ALLOCATOR_H

struct tracking_allocator : public allocator {
    allocator* Allocator = nullptr;
    // atom64     TotalAllocated = {0};
    // atom64     TotalFreed = {0};
    // atom64     CurrentAllocated = {0};

    tracking_allocator();
    tracking_allocator(allocator* _Allocator);

protected:
    void* Allocate_Internal(size_t Size) override;
    void  Free_Internal(void* Memory) override;
};

#endif