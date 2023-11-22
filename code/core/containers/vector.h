#ifndef VECTOR_H
#define VECTOR_H

template <typename type>
struct vector {
    allocator* Allocator = nullptr;
    type* Ptr            = nullptr;
    uptr  Capacity       = 0;
    uptr  Count          = 0;

    vector();
    vector(allocator* Allocator, u32 InitialCapacity=32);
    
    type& operator[](uptr Index);
    const type& operator[](uptr Index) const;

    void Push(const type& Entry);

    type& Last();
    const type& Last() const;
    
    void Reserve(uptr NewCapacity);
    void Clear();
    void Release();

    type* begin();
    type* end();

    type* begin() const;
    type* end() const;
};

#endif