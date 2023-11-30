inline memory_stream::memory_stream() { }
inline memory_stream::memory_stream(const const_buffer& Buffer) : Stream(Buffer.Ptr), Capacity(Buffer.Size), Used(0) { }

inline const void* memory_stream::Consume(size_t Size) {
    Assert(Used+Size <= Capacity, "Stream overflow!");
    const void* Result = Stream+Used;
    Used += Size;
    return Result;
}

inline u32 memory_stream::Consume_U32() {
    return *(u32*)Consume(sizeof(u32));
}

inline const void* memory_stream::Peek() const {
    return Stream+Used;
}

inline void memory_stream::Skip(size_t Size) {
    Assert(Used+Size <= Capacity, "Stream overflow!");
    Used += Size;
}

inline bool memory_stream::Is_Valid() const {
    return Used < Capacity;
}

inline void* Memory_Copy(void* Dst, const void* Src, uptr Size) {
    return memcpy(Dst, Src, Size);
}

inline void* Memory_Clear(void* Memory, uptr Size) {
    return memset(Memory, 0, Size);
}