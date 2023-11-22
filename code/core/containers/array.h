#ifndef ARRAY_H
#define ARRAY_H

struct array_find {
    bool Found;
    uptr Index;
};

template <typename type>
struct array {
    type* Ptr   = nullptr;
    uptr  Count = 0;

    array();
    array(type* _Ptr, uptr Count);
    array(allocator* Allocator, uptr Count);
    array(allocator* Allocator, const array& Array);
    
    type& operator[](uptr Index);
    const type& operator[](uptr Index) const;

    bool Empty() const;

    type& Random() const;
    array<type> Subdivide(uptr FirstIndex, uptr LastIndex);

    array_find Find_First(const function<bool(const type&)>& Find_Function);

    type* begin();
    type* end();

    type* begin() const;
    type* end() const;
};

#endif