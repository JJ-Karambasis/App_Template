#ifndef ARENA_H
#define ARENA_H

struct arena;

struct arena_block : buffer {
    uptr         Used;
    arena_block* Next;
};

struct arena_marker {
    arena*       Arena = nullptr;
    arena_block* CurrentBlock = nullptr;
    uptr         Marker = 0;
};

struct arena : public allocator {
    allocator*   Allocator = nullptr;
    arena_block* FirstBlock = nullptr;
    arena_block* LastBlock = nullptr;
    arena_block* CurrentBlock = nullptr;

    arena();
    arena(allocator* _Allocator);

    void* Push(uptr Size, uptr Alignment = 4, allocate_clear ClearFlag = DEFAULT_CLEAR_FLAG);
    arena_marker Get_Marker();
    void Set_Marker(arena_marker Marker);
    void Clear();
    void Release();

    template <typename type>
    inline type* Push_Struct(allocate_clear ClearFlag = DEFAULT_CLEAR_FLAG) { return (type*)Push(sizeof(type), alignof(type), ClearFlag); }

    template <typename type>
    inline type* Push_Array(u32 Count, allocate_clear ClearFlag = DEFAULT_CLEAR_FLAG) { return (type*)Push(sizeof(type)*Count, alignof(type), ClearFlag); }

protected:
    inline void* Allocate_Internal(size_t Size) override { return Push(Size, 1, allocate_clear::No_Clear); }
    inline void Free_Internal(void* Memory) override { /*Noop*/ }

private:
    arena_block* Get_Current_Block(uptr Size, uptr Alignment);
    arena_block* Allocate_Block(uptr BlockSize);
};

struct temp_arena : public allocator {
    arena_marker Marker;

    inline temp_arena(arena* Arena) : Marker(Arena->Get_Marker()) { }
    virtual inline ~temp_arena() {
        Marker.Arena->Set_Marker(Marker);
    }

    inline void* Push(size_t PushSize, size_t Alignment = 4, allocate_clear ClearFlag = DEFAULT_CLEAR_FLAG) {
        return Marker.Arena->Push(PushSize, Alignment, ClearFlag);
    }

protected:
    inline void* Allocate_Internal(size_t Size) override { return Push(Size, 1, allocate_clear::No_Clear); }
    inline void Free_Internal(void* Memory) override { /*Noop*/ }
public:

    template <typename type>
    inline type* Push_Struct(allocate_clear ClearFlag = DEFAULT_CLEAR_FLAG) { return (type*)Push(sizeof(type), alignof(type), ClearFlag); }

    template <typename type>
    inline type* Push_Array(u32 Count, allocate_clear ClearFlag = DEFAULT_CLEAR_FLAG) { return (type*)Push(sizeof(type)*Count, alignof(type), ClearFlag); }

    inline arena* Get_Arena() {
        return Marker.Arena;
    }
};


#endif