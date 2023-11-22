template <typename type>
inline array<type>::array() { }

template <typename type>
inline array<type>::array(type* _Ptr, uptr _Count) : Ptr(_Ptr), Count(_Count) { }

template <typename type>
inline array<type>::array(allocator* Allocator, uptr _Count) : Count(_Count) {
    Ptr = new(Allocator) type[Count];
}

template <typename type>
inline array<type>::array(allocator* Allocator, const array& Array) : Count(Array.Count) {
    Ptr = new(Allocator) type[Count];
    Memory_Copy(Ptr, Array.Ptr, Count*sizeof(type));
}

template <typename type>
inline type& array<type>::operator[](uptr Index) {
    Assert(Index < Count, "Array overflow!");
    return Ptr[Index];
}

template <typename type>
inline const type& array<type>::operator[](uptr Index) const {
    Assert(Index < Count, "Array overflow!");
    return Ptr[Index];
}

template <typename type>
bool array<type>::Empty() const {
    return Ptr == nullptr || Count == 0; 
}

template <typename type>
inline array<type> array<type>::Subdivide(uptr FirstIndex, uptr LastIndex) {
    Assert(FirstIndex < Count && LastIndex <= Count && FirstIndex <= LastIndex, "Invalid inputs");
    type* PtrAt = Ptr+FirstIndex;
    uptr NewCount = LastIndex-FirstIndex;
    return array<type>(PtrAt, NewCount);
}

template <typename type>
array_find array<type>::Find_First(const function<bool(const type&)>& Find_Function) {
    for(uptr i = 0; i < Count; i++) {
        if(Find_Function(Ptr[i])) {
            return {
                .Found = true,
                .Index = i
            };
        }
    }

    return {};
}

template <typename type>
inline type* array<type>::begin() {
    return Ptr;
}

template <typename type>
inline type* array<type>::end() {
    return Ptr + Count;
}

template <typename type>
inline type* array<type>::begin() const {
    return Ptr;
}

template <typename type>
inline type* array<type>::end() const {
    return Ptr + Count;
}