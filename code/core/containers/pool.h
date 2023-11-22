#ifndef POOL_H
#define POOL_H

union pool_id {
    u64 ID = 0;
    struct {
        u32 Generation;
        union {
            u32 Index; //When id is allocated, index is the current index in the pool
            u32 NextIndex; //When id is on free list, next index points to next free entry
        };
    };
};

template <typename type>
struct handle {
    pool_id ID = {};

    inline handle() { }
    inline handle(pool_id _ID) : ID(_ID) { }
    inline handle(u64 _ID) : ID({_ID}) { }

    inline bool Is_Null() const { return ID.ID == 0; }
};

template <typename type>
inline bool operator==(const handle<type>& A, const handle<type>& B) {
    return A.ID.ID == B.ID.ID;
}

template <typename type>
inline bool operator!=(const handle<type>& A, const handle<type>& B) {
    return A.ID.ID != B.ID.ID;
}

template <typename type>
struct pool {
    static const u32 INVALID = (u32)-1;

    struct entry {
        pool_id ID;
        type    Entry;
    };

    vector<entry> Entries;
    u32           FirstFreeIndex = INVALID;
    u32           Count = 0;

    pool();
    pool(allocator* Allocator);

    bool Is_Allocated(handle<type> Handle) const;
    
    type* Get(handle<type> Handle);
    const type* Get(handle<type> Handle) const;
    
    type& operator[](handle<type> Handle);
    const type& operator[](handle<type> Handle) const;

    handle<type> Allocate();
    void Free(handle<type> Handle);

    void Clear();
    void Release();

    struct pool_iter {
        pool* Pool;
        u32   Index;

        type& operator*();
        bool operator!=(const pool_iter& Other) const;
        pool_iter& operator++();
    };

    pool_iter begin();
    pool_iter end();

    struct const_pool_iter {
        const pool* Pool;
        u32         Index;

        const type& operator*() const;
        bool operator!=(const const_pool_iter& Other) const;
        const_pool_iter& operator++();
    };

    const_pool_iter begin() const;
    const_pool_iter end() const;

    struct handle_iter {
        const pool* Pool;

        struct internal_handle_iter {
            const pool* Pool;
            u32 Index;

            handle<type> operator*() const;
            bool operator!=(const internal_handle_iter& Other) const;
            internal_handle_iter& operator++();
        };

        internal_handle_iter begin() const;
        internal_handle_iter end() const;
    };

    handle_iter Handle_Iter() const;
};

#endif