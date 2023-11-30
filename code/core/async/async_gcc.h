#ifndef ASYNC_GCC_H
#define ASYNC_GCC_H

#define Atomic_Fence_Acq() asm volatile("" ::: "memory")
#define Atomic_Fence_Rel() asm volatile("" ::: "memory")

struct atom32 {
    volatile u32 Value;
} __attribute__((aligned(4)));

inline u32 Atomic_Load(atom32* Object) {
    return Object->Value;
}

inline void Atomic_Store(atom32* Object, u32 Value) {
    Object->Value = Value;
}

inline bool Atomic_Compare_Exchange(atom32* Object, u32 OldValue, u32 NewValue) {
    return __sync_bool_compare_and_swap((u32*)Object, OldValue, NewValue);
}

inline u32 Atomic_Add(atom32* Object, s32 Addend) {
    return (u32)__sync_fetch_and_add((s32*)Object, Addend);
}

inline u32 Atomic_Increment(atom32* Object) {
    return Atomic_Add(Object, 1) + 1;
}

inline u32 Atomic_Decrement(atom32* Object) {
    return Atomic_Add(Object, -1) - 1;
}

#endif