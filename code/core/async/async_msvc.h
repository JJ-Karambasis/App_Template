#ifndef ASYNC_MSVC_H
#define ASYNC_MSVC_H

//-------------------------------------
//  Atomic types
//-------------------------------------
// In MSVC, correct alignment of each type is already ensured.
// MSVC doesn't seem subject to out-of-thin-air stores like GCC, so volatile is
// omitted.
// (MS volatile implies acquire & release semantics, which may be expensive on
// ARM or Xbox 360.)

#define Atomic_Fence_Acq() _ReadWriteBarrier()
#define Atomic_Fence_Rel() _ReadWriteBarrier()

struct atom32 {
    u32 Value;
};

inline u32 Atomic_Load(atom32* Object) {
    // Do a volatile load so that compiler doesn't duplicate loads
    return ((volatile atom32*)Object)->Value;
}

inline void Atomic_Store(atom32* Object, u32 Value) {
    ((volatile atom32*)Object)->Value = Value;
}

inline bool Atomic_Compare_Exchange(atom32* Object, u32 OldValue, u32 NewValue) {
    return (u32)_InterlockedCompareExchange((long*)Object, (long)NewValue, (long)OldValue) == OldValue;
}

inline u32 Atomic_Add(atom32* Object, s32 Addend) {
    return (u32)_InterlockedExchangeAdd((long*)Object, (long)Addend);
}

inline u32 Atomic_Increment(atom32* Object) {
    return (u32)_InterlockedIncrement((long*)Object);
}

inline u32 Atomic_Decrement(atom32* Object) {
    return (u32)_InterlockedDecrement((long*)Object);
}

struct atom64 {
    u64 Value;
};

#if defined(ENVIRONMENT32)

inline u64 Atomic_Load(atom64* Object) {
    return (u64)_InterlockedCompareExchange64((__int64*)&Object->Value, 0, 0);
}

inline void Atomic_Store(atom64* Object, u64 Value) {
    u64 OldValue;
    do {
        OldValue = Object->Value;
    } while(_InterlockedCompareExchange64((__int64*)&Object->Value, (__int64)Value, (__int64)OldValue) == OldValue);
}

inline bool Atomic_Compare_Exchange(atom64* Object, u64 OldValue, u64 NewValue) {
    return (u64)_InterlockedCompareExchange64((__int64*)Object, (__int64)NewValue, (__int64)OldValue) == OldValue;
}

inline u64 Atomic_Add(atom64* Object, s64 Addend) {
    for(;;) {
        s64 OldValue = (s64)Object->Value;
        if(Atomic_Compare_Exchange(Object, OldValue, OldValue+Addend))
            return OldValue;
    }
}

inline u64 Atomic_Increment(atom64* Object) {
    return Atomic_Add(Object, 1) + 1;
}

inline u64 Atomic_Decrement(atom64* Object) {
    return Atomic_Add(Object, -1) - 1;
}

#elif defined(ENVIRONMENT64)

inline u64 Atomic_Load(atom64* Object) {
    // Do a volatile load so that compiler doesn't duplicate loads, which makes
    // them nonatomic. (Happened in testing.)
    return ((volatile atom64*) Object)->Value;
}

inline void Atomic_Store(atom64* Object, u64 Value) {
    // Do a volatile load so that compiler doesn't duplicate loads, which makes
    // them nonatomic. (Happened in testing.)
    ((volatile atom64*)Object)->Value = Value;
}

inline bool Atomic_Compare_Exchange(atom64* Object, u64 OldValue, u64 NewValue) {
    return (u64)_InterlockedCompareExchange64((__int64*)Object, (__int64)NewValue, (__int64)OldValue) == OldValue;
}

inline u64 Atomic_Add(atom64* Object, s64 Addend) {
    return (u64)_InterlockedExchangeAdd64((__int64*)Object, (__int64)Addend);
}

inline u64 Atomic_Increment(atom64* Object) {
    return (u64)_InterlockedIncrement64((LONGLONG*)Object);
}

inline u64 Atomic_Decrement(atom64* Object) {
    return (u64)_InterlockedDecrement64((LONGLONG*)Object);
}

#else
#error Invalid environment!
#endif

#endif