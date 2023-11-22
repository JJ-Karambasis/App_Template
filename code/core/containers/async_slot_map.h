#ifndef ASYNC_SLOT_MAP_H
#define ASYNC_SLOT_MAP_H

//TODO: Atomic stores here
struct async_stack_index {
    static const u32 INVALID_INDEX = (u32)-1;
    union stack_index {
        atom64 ID;
        struct {
            atom32 Index;
            atom32 Key;
        };

        inline stack_index() {
            Atomic_Store(&Index, INVALID_INDEX);
            Atomic_Store(&Key, 0);
        }

        inline stack_index(u32 _Index, u32 _Key) {
            Atomic_Store(&Index, _Index);
            Atomic_Store(&Key, _Key);
        }
    };

    allocator*   Allocator = nullptr;
    array<u32>   Indices;
    stack_index  Top;

#ifdef ASYNC_SLOT_MAP_STATS
    struct {
        atom32 PushCount;
        atom32 PopCount;
    } Stats = {};
#endif

    inline async_stack_index() { }
    inline async_stack_index(allocator* _Allocator, u32 Count) : 
        Allocator(_Allocator), Indices(Allocator, Count) { }

    inline void Push(u32 Index) {
        for(;;) {
            stack_index CurrentTop = Top;
            u32 Current = CurrentTop.Index.Value;
            Indices[Index] = Current;
            stack_index NewTop(Index, CurrentTop.Key.Value+1);

            if(Atomic_Compare_Exchange(&Top.ID, CurrentTop.ID.Value, NewTop.ID.Value)) {
                break;
            }
        }

#ifdef ASYNC_SLOT_MAP_STATS
        Atomic_Increment(&Stats.PushCount);
#endif
    }

    inline u32 Pop() {
        for(;;) {
            stack_index CurrentTop = Top;
            u32 Result = CurrentTop.Index.Value;
            if(Result == INVALID_INDEX) {
                return INVALID_INDEX;
            }

            u32 Next = Indices[Result];
            stack_index NewTop(Next, CurrentTop.Key.Value+1);

            if(Atomic_Compare_Exchange(&Top.ID, CurrentTop.ID.Value, NewTop.ID.Value)) {
#ifdef ASYNC_SLOT_MAP_STATS
                Atomic_Increment(&Stats.PopCount);
#endif
                return Result;
            }
        }
    }

    inline void Release() {
        if(!Indices.Empty()) {
            Allocator->Free(Indices.Ptr);
            Indices = {};
        }
    }
};

union async_slot_id {
    atom64 ID = {};
    struct {
        atom32 Index;
        atom32 Generation;
    };

    inline async_slot_id(u32 _Index, u32 _Generation) {
        Atomic_Store(&Index, _Index);
        Atomic_Store(&Generation, _Generation);
    }
};

template <typename type>
struct async_handle {
    async_slot_id ID;

    inline async_handle() { }
    
    inline async_handle(async_slot_id _ID) : ID(_ID) { }
    
    inline async_handle(u64 _ID) { 
        Atomic_Store(&ID.ID, _ID);
    }

    inline bool Is_Null() const { return Atomic_Load(&ID.ID) == 0; }
    inline u32 Get_Index() const { return Atomic_Load(&ID.Index); }
};

template <typename type>
struct async_slot_map {
    allocator*           Allocator;
    array<async_slot_id> Slots;
    async_stack_index    FreeIndices;

#ifdef ASYNC_SLOT_MAP_STATS
    struct {
        atom32 AllocatedCount;
        atom32 FreedCount;
    } Stats = {};
#endif

    inline async_slot_map() { }
    inline async_slot_map(allocator* _Allocator, u32 MaxCount) : 
        Allocator(_Allocator), Slots(Allocator, MaxCount), FreeIndices(Allocator, MaxCount) { 
        for(u32 i = 0; i < MaxCount; i++) {
            FreeIndices.Push(i);
            Slots[i] = async_slot_id(i, 1);
        }
    }

    inline bool Is_Allocated(async_handle<type> Handle) {
        if(Handle.Is_Null()) return false;
        u32 Index = Handle.Get_Index();
        return Handle.ID.Generation == Slots[Index].Generation;
    }

    inline async_handle<type> Allocate() {
        u32 Index = FreeIndices.Pop();
        if(Index == async_stack_index::INVALID_INDEX) {
            return {};
        }
        
#ifdef ASYNC_SLOT_MAP_STATS
        Atomic_Increment(&Stats.AllocatedCount);
#endif
        return async_handle<type>(Slots[Index]);
    }

    inline void Free(async_handle<type> Handle) {
        if(Handle.Is_Null()) return;

        u32 Index = Handle.Get_Index();
        if(Handle.ID.Generation == Slots[Index].Generation) {
            Atomic_Store(&Slots[Index].Generation, Atomic_Load(&Slots[Index].Generation)+1);
            
            Atomic_Fence_Rel();
            
            FreeIndices.Push(Atomic_Load(&Slots[Index].Index));
#ifdef ASYNC_SLOT_MAP_STATS
            Atomic_Increment(&Stats.FreedCount);
#endif
        }
    }

    inline void Release() {
        FreeIndices.Release();

        if(!Slots.Empty()) {
            Allocator->Free(Slots.Ptr);
            Slots = {};
        }
    }
};

#endif