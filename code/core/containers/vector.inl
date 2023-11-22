
template <typename type>
inline vector<type>::vector() { }

template <typename type>
inline vector<type>::vector(allocator* _Allocator, u32 InitialCapacity) : Allocator(_Allocator) {
    Reserve(InitialCapacity);
}

template <typename type>
inline type& vector<type>::operator[](uptr Index) {
    Assert(Index < Count, "Vector overflow!");
    return Ptr[Index];
}

template <typename type>
inline const type& vector<type>::operator[](uptr Index) const {
    Assert(Index < Count, "Vector overflow!");
    return Ptr[Index];
}

template <typename type>
inline void vector<type>::Push(const type& Entry) {
    if(Count == Capacity) {
        Reserve(Capacity*2);
    }
    Ptr[Count++] = Entry;
}

template <typename type>
type& vector<type>::Last() {
    Assert(Count, "Vector has no entries");
    return Ptr[Count-1];
}

template <typename type>
const type& vector<type>::Last() const {
    Assert(Count, "Vector has no entries");
    return Ptr[Count-1];
}

template <typename type>
inline void vector<type>::Reserve(uptr Size) {
    uptr NewCapacity = Size;
    type* NewData = new(Allocator) type[NewCapacity];

    if(Ptr) {
        //Just in case the vector is smaller, we need to check to see which capacity
        //we will copy against
        uptr CopySize = Capacity > NewCapacity ? NewCapacity : Capacity;
        Memory_Copy(NewData, Ptr, CopySize*sizeof(type));
        operator delete[](Ptr, Allocator);
    }

    Ptr = NewData;
    Capacity = NewCapacity;

    //Clamp the count to make sure we don't fill over
    if(Capacity < Count)
        Count = Capacity;
}

template <typename type>
inline void vector<type>::Clear() {
    Count = 0;
}

template <typename type>
inline void vector<type>::Release() {
    Count = 0;
    Capacity = 0;

    if(Ptr) {
        operator delete[](Ptr, Allocator);
        Ptr = nullptr;
    }
}

template <typename type>
inline type* vector<type>::begin() {
    return Ptr;
}

template <typename type>
inline type* vector<type>::end() {
    return Ptr + Count;
}

template <typename type>
inline type* vector<type>::begin() const {
    return Ptr;
}

template <typename type>
inline type* vector<type>::end() const {
    return Ptr + Count;
}