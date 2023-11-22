template <typename type>
inline span<type>::span() { }

template <typename type>
inline span<type>::span(const type* _Ptr, uptr _Count) : Ptr(_Ptr), Count(_Count) { }

template <typename type>
inline span<type>::span(const vector<type>& Vector) : Ptr(Vector.Ptr), Count(Vector.Count) { }

template <typename type>
inline span<type>::span(std::initializer_list<type> List) : Ptr(List.begin()), Count(List.size()) { }

template <typename type>
inline const type& span<type>::operator[](uptr Index) const {
    Assert(Index < Count, "Span overflow!");
    return Ptr[Index];
}

template <typename type>
inline const type* span<type>::begin() const {
    return Ptr;
}

template <typename type>
inline const type* span<type>::end() const {
    return Ptr + Count;
}