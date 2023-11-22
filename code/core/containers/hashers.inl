
inline void Hash_Combine(u32& seed) { }

template <typename type, typename... rest, typename hasher=hasher<type>>
inline void Hash_Combine(u32& Seed, const type& Value, rest... Rest) {
    Seed ^= hasher{}.Hash(Value) + 0x9e3779b9 + (Seed << 6) + (Seed >> 2);
    Hash_Combine(Seed, Rest...); 
}

template <typename type, typename... rest, typename hasher=hasher<type>>
inline u32 Hash_Combine(const type& Value, rest... Rest) {
    u32 Result = hasher{}.Hash(Value);
    Hash_Combine(Result, Rest...);
    return Result;
}

template <>
struct hasher<u32> {
    inline u32 Hash(u32 Key) {
        Key = (Key+0x7ed55d16) + (Key<<12);
        Key = (Key^0xc761c23c) ^ (Key>>19);
        Key = (Key+0x165667b1) + (Key<<5);
        Key = (Key+0xd3a2646c) ^ (Key<<9);
        Key = (Key+0xfd7046c5) + (Key<<3);
        Key = (Key^0xb55a4f09) ^ (Key>>16);
        return Key;
    }
};

template <>
struct comparer<u32> {
    inline bool Equal(u32 A, u32 B) {
        return A == B;
    }
};

template <>
struct hasher<u64> {
    inline u32 Hash(u64 Key) {
        Key = (~Key) + (Key << 18); // Key = (Key << 18) - Key - 1;
        Key = Key ^ (Key >> 31);
        Key = Key * 21; // Key = (Key + (Key << 2)) + (Key << 4);
        Key = Key ^ (Key >> 11);
        Key = Key + (Key << 6);
        Key = Key ^ (Key >> 22);
        return (u32)Key;
    }
};

template <>
struct comparer<u64> {
    inline bool Equal(u64 A, u64 B) {
        return A == B;
    }
};

template <>
struct hasher<s32> {
    inline u32 Hash(s32 Key) {
        return hasher<u32>{}.Hash((u32)Key);
    }
};

template <>
struct comparer<s32> {
    inline bool Equal(s32 A, s32 B) {
        return A == B;
    }
};

template <>
struct hasher<s64> {
    inline u32 Hash(s64 Key) {
        return hasher<u64>{}.Hash((u64)Key);
    }
};

template <>
struct comparer<s64> {
    inline bool Equal(s64 A, s64 B) {
        return A == B;
    }
};

template <>
struct hasher<string> {
    inline u32 Hash(const string& Str) {
        u64 Result = 0;
        u64 Rand1 = 31414;
        u64 Rand2 = 27183;
        
        for(u64 Index = 0; Index < Str.Size; Index++)
        {
            Result *= Rand1;
            Result += Str.Str[Index];
            Rand1 *= Rand2;
        }
        
        return hasher<u64>{}.Hash(Result);
    }
};

template <>
struct comparer<string> {
    inline bool Equal(const string& A, const string& B) {
        return A == B;
    }
};

template <>
struct hasher<void*> {
    inline u32 Hash(const void* Ptr) {
#if defined(ENVIRONMENT32)
        return hasher<u32>{}.Hash((u32)Ptr);
#elif defined(ENVIRONMENT64)
        return hasher<u64>{}.Hash((u64)Ptr);
#else
#error Invalid environment!
#endif
    }
};

template <>
struct comparer<void*> {
    inline bool Equal(const void* A, const void* B) {
        return A == B;
    }
};