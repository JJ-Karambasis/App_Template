template <typename type>
inline pool<type>::pool() { }

template <typename type>
inline pool<type>::pool(allocator* Allocator) : Entries(Allocator) {}

template <typename type>
inline bool pool<type>::Is_Allocated(handle<type> Handle) const {
    if(Handle.Is_Null()) return false;
    const pool<type>::entry& Entry = Entries[Handle.ID.Index];
    return Entry.ID.Generation == Handle.ID.Generation;
}

template <typename type>
inline type* pool<type>::Get(handle<type> Handle) {
    if(!Is_Allocated(Handle)) return nullptr;
    return &Entries[Handle.ID.Index].Entry;
}

template <typename type>
inline const type* pool<type>::Get(handle<type> Handle) const {
    if(!Is_Allocated(Handle)) return nullptr;
    return &Entries[Handle.ID.Index].Entry;
}

template <typename type>
inline type& pool<type>::operator[](handle<type> Handle) {
    Assert(Is_Allocated(Handle), "Invalid handle!");
    return Entries[Handle.ID.Index].Entry;
}

template <typename type>
inline const type& pool<type>::operator[](handle<type> Handle) const {
    Assert(Is_Allocated(Handle), "Invalid handle!");
    return Entries[Handle.ID.Index].Entry;
}

template <typename type>
inline handle<type> pool<type>::Allocate() {
    u32 Index = 0;
    if(FirstFreeIndex == INVALID) {
        Index = Safe_U32(Entries.Count);
        Entries.Push({});
    } else {
        Index = FirstFreeIndex;
        FirstFreeIndex = Entries[FirstFreeIndex].ID.NextIndex;
    }

    entry& Entry = Entries[Index];
    Entry.ID.Index = Index;

    Set_Bit(Entry.ID.Generation, 31);

    handle<type> Result = {Entry.ID};
    Count++;
    return Result;
}

template <typename type>
inline void pool<type>::Free(handle<type> Handle) {
    if(Handle.Is_Null()) return;

    entry& Entry = Entries[Handle.ID.Index];
    if(Entry.ID.Generation == Handle.ID.Generation) {
        Assert(Get_Bit(Entry.ID.Generation, 31), "Entry has already been freed!");
        Entry.ID.Generation++;
        Clear_Bit(Entry.ID.Generation, 31);

        //We ran out of unique keys, reset back to 1
        if(!Entry.ID.Generation) {
            Entry.ID.Generation = 1;
        }

        Entry.ID.NextIndex = FirstFreeIndex;
        FirstFreeIndex = Handle.ID.Index;
        Count--;
    }
}

template <typename type>
inline void pool<type>::Clear() {
    FirstFreeIndex = INVALID;
    Count = 0;
    Entries.Clear();
}

template <typename type>
inline void pool<type>::Release() {
    FirstFreeIndex = INVALID;
    Count = 0;
    Entries.Release();
}

template <typename type>
inline type& pool<type>::pool_iter::operator*() {
    return Pool->Entries[Index].Entry;
}

template <typename type>
inline bool pool<type>::pool_iter::operator!=(const pool<type>::pool_iter& Other) const {
    return Index != INVALID;
}

template <typename type>
inline typename pool<type>::pool_iter& pool<type>::pool_iter::operator++() {
    u32 EntryIndex = Index+1;
    Index = INVALID;
    for(; EntryIndex < Pool->Entries.Count; EntryIndex++) {
        if(Get_Bit(Pool->Entries[EntryIndex].ID.Generation, 31)) {
            Index = EntryIndex;
            break;
        }
    }

    return *this;
}

template <typename type>
inline typename pool<type>::pool_iter pool<type>::begin() {
    pool<type>::pool_iter Result = {};
    Result.Index = INVALID;

    for(u32 EntryIndex = 0; EntryIndex < Entries.Count; EntryIndex++) {
        if(Get_Bit(Entries[EntryIndex].ID.Generation, 31)) {
            Result.Index = EntryIndex;
            Result.Pool = this;
            break;
        }
    }

    return Result;
}

template <typename type>
inline typename pool<type>::pool_iter pool<type>::end() {
    pool<type>::pool_iter Result = {};
    return Result;
}

template <typename type>
inline const type& pool<type>::const_pool_iter::operator*() const {
    return Pool->Entries[Index].Entry;
}

template <typename type>
inline bool pool<type>::const_pool_iter::operator!=(const pool<type>::const_pool_iter& Other) const {
    return Index != INVALID;
}

template <typename type>
inline typename pool<type>::const_pool_iter& pool<type>::const_pool_iter::operator++() {
    u32 EntryIndex = Index+1;
    Index = INVALID;
    for(; EntryIndex < Pool->Entries.Count; EntryIndex++) {
        if(Get_Bit(Pool->Entries[EntryIndex].ID.Generation, 31)) {
            Index = EntryIndex;
            break;
        }
    }

    return *this;
}

template <typename type>
inline typename pool<type>::const_pool_iter pool<type>::begin() const {
    pool<type>::const_pool_iter Result = {};
    Result.Index = INVALID;

    for(u32 EntryIndex = 0; EntryIndex < Entries.Count; EntryIndex++) {
        if(Get_Bit(Entries[EntryIndex].ID.Generation, 31)) {
            Result.Index = EntryIndex;
            Result.Pool = this;
            break;
        }
    }

    return Result;
}

template <typename type>
inline typename pool<type>::const_pool_iter pool<type>::end() const {
    pool<type>::const_pool_iter Result = {};
    return Result;
}

template <typename type>
inline handle<type> pool<type>::handle_iter::internal_handle_iter::operator*() const {
    return handle<type>(Pool->Entries[Index].ID);
}

template <typename type>
inline bool pool<type>::handle_iter::internal_handle_iter::operator!=(const pool<type>::handle_iter::internal_handle_iter& Other) const {
    return Index != INVALID;
}

template <typename type>
inline typename pool<type>::handle_iter::internal_handle_iter& pool<type>::handle_iter::internal_handle_iter::operator++() {
    u32 EntryIndex = Index+1;
    Index = INVALID;
    for(; EntryIndex < Pool->Entries.Count; EntryIndex++) {
        if(Get_Bit(Pool->Entries[EntryIndex].ID.Generation, 31)) {
            Index = EntryIndex;
            break;
        }
    }

    return *this;
}

template <typename type>
inline typename pool<type>::handle_iter::internal_handle_iter pool<type>::handle_iter::begin() const {
    pool<type>::handle_iter::internal_handle_iter Result = {};
    Result.Index = INVALID;

    for(u32 EntryIndex = 0; EntryIndex < Pool->Entries.Count; EntryIndex++) {
        if(Get_Bit(Pool->Entries[EntryIndex].ID.Generation, 31)) {
            Result.Index = EntryIndex;
            Result.Pool = Pool;
            break;
        }
    }

    return Result;
}

template <typename type>
inline typename pool<type>::handle_iter::internal_handle_iter pool<type>::handle_iter::end() const {
    pool<type>::handle_iter::internal_handle_iter Result = {};
    return Result;
}

template <typename type>
inline typename pool<type>::handle_iter pool<type>::Handle_Iter() const {
    pool<type>::handle_iter Result = {this};
    return Result;
}