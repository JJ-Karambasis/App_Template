template <typename type, uptr N>
type& static_array<type, N>::operator[](uptr Index) {
    Assert(Index < N, "Array overflow!");
    return Ptr[Index];
}

template <typename type, uptr N>
const type& static_array<type, N>::operator[](uptr Index) const {
    Assert(Index < N, "Array overflow!");
    return Ptr[Index];
}