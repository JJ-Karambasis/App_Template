#ifndef SPAN_H
#define SPAN_H

template <typename type>
struct vector;

template <typename type>
struct span {
    const type* Ptr  = nullptr;
    uptr        Count = 0;

    span();
    span(const type* _Ptr, uptr Count);
    span(const vector<type>& Vector);
    span(std::initializer_list<type> List);
    const type& operator[](uptr Index) const;

    const type* begin() const;
    const type* end() const;
};

#endif