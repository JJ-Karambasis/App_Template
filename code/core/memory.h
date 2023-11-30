#ifndef MEMORY_H
#define MEMORY_H

struct buffer {
    u8*  Ptr;
    uptr Size;

    inline buffer() { }
    inline buffer(void* Data, uptr BufferSize) : Ptr((u8*)Data), Size(BufferSize) { }
};

struct const_buffer {
    const u8* Ptr  = nullptr;
    uptr      Size = 0;

    inline const_buffer() { }
    
    inline const_buffer(const void* Data, uptr BufferSize) : Ptr((const u8*)Data), Size(BufferSize) { }

    template <typename type, uptr N>
    inline const_buffer(const type (&Array)[N]) : Ptr((const u8*)Array), Size(sizeof(type)*N) { }

    inline bool Is_Empty() const {
        return !Size || !Ptr;
    }
};

struct memory_stream {
    const u8* Stream = nullptr;
    size_t    Capacity = 0;
    size_t    Used = 0;

    memory_stream();
    memory_stream(const const_buffer& Buffer);

    const void* Consume(size_t Size);
    u32         Consume_U32();
    const void* Peek() const;
    void        Skip(size_t Size);
    bool        Is_Valid() const;
};

void* Memory_Copy(void* Dst, const void* Src, uptr Size);
void* Memory_Clear(void* Memory, uptr Size);

template <typename type>
inline type* Zero_Struct(type* Ptr) {
    return (type*)Memory_Clear(Ptr, sizeof(type));
}

template <typename type>
inline type* Zero_Array(type* Ptr, uptr Count) {
    return (type*)Memory_Clear(Ptr, sizeof(type)*Count);
}

template <typename type>
inline type* Copy_Array(type* Dst, const type* Src, uptr Count) {
    return (type*)Memory_Copy(Dst, Src, sizeof(type)*Count);
}


#endif