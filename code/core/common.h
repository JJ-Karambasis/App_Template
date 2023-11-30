#ifndef COMMON_H
#define COMMON_H

struct allocator;

/// @brief: A custom operator new allocation function to create function invokers
/// @param Size: The size of the function invoker we want to create
/// @param Buffer: A buffer to hold a small function callbacks data
/// @return: A pointer to the newly allocated function invoker
inline void* operator new(size_t Size, buffer* Buffer) {
    if(Size <= Buffer->Size) return Buffer->Ptr;
    return ::operator new(Size);
}

/// @brief: A custom operator delete allocation function to delete function invokers
/// @param Memory: The function invoker memory to delete
/// @param Buffer: A buffer that holds a small function callbacks data
inline void operator delete(void* Memory, buffer* Buffer) noexcept   {
    if(Buffer->Ptr != Memory) {
        ::operator delete(Memory);
    }
}

inline u32 Ceil_Pow2(u32 V) {
    V--;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    V++;
    return V;
}

inline u64 Ceil_Pow2(u64 V) {
    V--;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    V |= V >> 32;
    V++;
    return V;
}

inline char To_Lower(char C) {
    return (char)tolower(C);
}

inline char To_Upper(char C) {
    return (char)toupper(C);
}

inline u32 Safe_U32(u64 x) {
    Assert(x <= 0xFFFFFFFF, "Integer overflow!");
    return (u32)x;
}

inline f32 SNorm(s16 Value) {
    return Clamp(-1.0f, (f32)Value / (f32)((1 << 15) - 1), 1.0f);
}

inline s16 SNorm_S16(f32 Value) {
    s16 Result = (s16)(Clamp(-1.0f, Value, 1.0f) * (f32)(1 << 15));
    return Result;
}

inline u32 Ceil_U32(f64 V) {
    return (u32)ceil(V);
}

inline f32 Equal_Zero_Eps_Sq(f32 SqValue) {
    return Abs(SqValue) <= Sq(FLT_EPSILON);
}

inline f32 Sqrt(f32 Value) {
    return sqrtf(Value);
}

inline f32 Cos(f32 Angle) {
    return cosf(Angle);
}

inline f32 Sin(f32 Angle) {
    return sinf(Angle);
}

inline s32 Random_Between(s32 Min, s32 Max) {
    s32 n = Max - Min + 1;
    s32 remainder = RAND_MAX % n;
    s32 x;
    do{
        x = rand();
    } while (x >= RAND_MAX - remainder);
    return Min + x % n;
}

template <typename type> 
struct range_iter {
    type Current;
    
    inline range_iter(const type& _Current) : Current(_Current) { }
    inline type operator*() const { return Current; }
    inline type const* operator->() { return &Current; }
    inline range_iter& operator++() { Current++; return *this; }
    inline bool operator==(const range_iter& Other) const { return Current == Other.Current; }
    inline bool operator!=(const range_iter& Other) const { return Current != Other.Current; }
};

template <typename type> 
struct range_proxy {
    range_iter<type> Begin;
    range_iter<type> End;
    
    inline range_proxy(const type& _Begin, const type& _End) : Begin(_Begin), End(_End) { }
    inline range_iter<type> begin() const { return Begin; }
    inline range_iter<type> end() const { return End; }
};

template <typename type> 
inline auto Indices(const type& Type) -> range_proxy<decltype(Type.Count)> { return {0, Type.Count}; }

template <typename type, uptr N>
inline auto Indices(const type (&Arr)[N]) -> range_proxy<decltype(N)> { return {0, N}; }

//Forward declare function parameter
template <typename>
struct function;

/// @brief: Function invoke base class with custom arguments
/// @tparam return_type: The return type for the custom function callback
/// @tparam ...args: The additional arguments to specify for the custom function
template <typename return_type, typename... args>
struct function_invoker_base {
    
    /// @brief: Base virtual destructor
    virtual inline ~function_invoker_base() { }
    
    /// @brief: Invokes the function callback with the specified Args
    /// @param ...Args: Arguments to pass into the function callback
    /// @return: Returns the return type of the function callback
    virtual return_type Invoke(args... Args) const  = 0;

    /// @brief: Clones the function invoker
    /// @param Buffer: A buffer to hold small function callback data 
    /// @return: A newly cloned function invoker
    virtual function_invoker_base* Clone(buffer* Buffer) const = 0;
};

/// @brief: The main function invoker class
/// @tparam functor: The function object that contains the captured variables and callback
/// @tparam return_type: The function callback return type
/// @tparam ...args: The additional arguments to specify for the custom function
template <typename functor, typename return_type, typename... args>
struct function_invoker : public function_invoker_base<return_type, args...>{
    functor Functor;

    /// @brief: Main constructor for a function invoker. Takes in a function object.
    /// @param _Functor: The function object to store.
    inline function_invoker(const functor& _Functor) : Functor(_Functor) { }

    /// @brief: Executes the function object callback with all the required arguments
    /// @param ...Args: The additional arguments for the function callback
    /// @return: The return type for the function callback
    inline return_type Invoke(args... Args) const final { return Functor(Args...); }

    /// @brief: Clones the function invoker
    /// @param Buffer: A buffer to hold small function callback data 
    /// @return: A newly cloned function invoker
    inline function_invoker_base<return_type, args...>* Clone(buffer* Buffer) const override {
        function_invoker_base<return_type, args...>* Result = new(Buffer) function_invoker<functor, return_type, args...>(Functor);
        return Result;
    }

    /// @brief Function invoker destructor
    virtual inline ~function_invoker() override { }

    function_invoker& operator=(const function_invoker&) = delete;
};

/// @brief: The main function invoker class
/// @tparam return_type: The function callback return type
/// @tparam ...args: The additional arguments to specify for the custom function
template <typename return_type, typename... args>
struct function_invoker_args : public function_invoker_base<return_type, args...>{
    return_type (*Functor)(args...);

    /// @brief: Main constructor for a function invoker. Takes in a function object.
    /// @param _Functor: The function object to store.
    inline function_invoker_args(return_type (*_Functor)(args...)) : Functor(_Functor) { }

    /// @brief: Executes the function object callback with all the required arguments
    /// @param ...Args: The additional arguments for the function callback
    /// @return: The return type for the function callback
    inline return_type Invoke(args... Args) const final { return Functor(Args...); }

    /// @brief: Clones the function invoker
    /// @param Buffer: A buffer to hold small function callback data 
    /// @return: A newly cloned function invoker
    inline function_invoker_base<return_type, args...>* Clone(buffer* Buffer) const override {
        function_invoker_base<return_type, args...>* Result = new(Buffer) function_invoker_args<return_type, args...>(Functor);
        return Result;
    }

    /// @brief Function invoker destructor
    virtual inline ~function_invoker_args() override { }

    function_invoker_args& operator=(const function_invoker_args&) = delete;
};

/// @brief: The main concrete implementation for custom function callbacks with variable captures via functors
/// @tparam return_type: The function callback return type
/// @tparam ...args: The additional argument to specify for the custom function
template <typename return_type, typename... args>
struct function<return_type(args...)> {
    
    static const u32 SMALL_DATA_SIZE = 16;
    function_invoker_base<return_type, args...>* Invoker = nullptr;
    u8 Data[SMALL_DATA_SIZE];
    buffer Buffer;

    /// @brief: Base constructor
    inline function() : Buffer(Data, SMALL_DATA_SIZE) { }

    inline function(return_type (*Functor)(args...)) : Buffer(Data, SMALL_DATA_SIZE) {
        Invoker = new(&Buffer) function_invoker_args<return_type, args...>(Functor);
    }
    
    /// @brief: Constructor with a custom functor object
    /// @tparam functor: The custom functor class defintion
    /// @param Functor: The custom functor object
    template <typename functor> 
    inline function(const functor& Functor) : Buffer(Data, SMALL_DATA_SIZE) {
        Invoker = new(&Buffer) function_invoker<functor, return_type, args...>(Functor);
    }

    /// @brief: Calls the function object callback
    /// @param ...Args: The arguments to specify for the function object
    /// @return: The return type of the function callback
    inline return_type operator()(args... Args) const {
        return Invoker->Invoke(Args...);
    }

    /// @brief: Copy constructor for the function
    /// @param Other: The function to copy
    inline function(const function& Other) {
        Invoker = Other.Invoker ? Other.Invoker->Clone(&Buffer) : nullptr;
    }

    /// @brief: Assigned operator for the function callback
    /// @param Other: The function to copy
    /// @return: A newly copied function
    inline function& operator=(const function& Other) {
        Invoker = Other.Invoker ? Other.Invoker->Clone(&Buffer) : nullptr;
        return *this;
    }
    
    /// @brief: Function destructor
    inline ~function() {
        if(Invoker) {
            ::operator delete(Invoker, &Buffer);
        }
    }
};

#endif