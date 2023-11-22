inline void* Memory_Copy(void* Dst, const void* Src, uptr Size) {
    return memcpy(Dst, Src, Size);
}

inline void* Memory_Clear(void* Memory, uptr Size) {
    return memset(Memory, 0, Size);
}