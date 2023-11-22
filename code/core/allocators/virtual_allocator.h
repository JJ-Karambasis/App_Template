#ifndef VIRTUAL_ALLOCATOR_H
#define VIRTUAL_ALLOCATOR_H

struct virtual_allocator : public allocator {
    void* Reserve(uptr Size);
    void* Commit(void* Memory, uptr Size);
    void  Decommit(void* Memory, uptr Size);
    void  Release(void* Memory, uptr Size);

protected:
    virtual void* Allocate_Internal(uptr Size) override;
    virtual void  Free_Internal(void* Memory) override;
};

#endif