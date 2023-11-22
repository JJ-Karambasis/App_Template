#ifndef STATIC_ARRAY_H
#define STATIC_ARRAY_H

template <typename type, uptr N>
struct static_array {
    type Ptr[N];

    type& operator[](uptr Index);
    const type& operator[](uptr Index) const;

    inline constexpr uptr Count() const { return N; }
};

#endif