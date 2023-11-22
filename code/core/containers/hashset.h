#ifndef HASHSET_H
#define HASHSET_H

template <typename key, typename hasher = hasher<key>, typename comparer = comparer<key>>
struct hashset {
    static const u32 INVALID = hash_slot::INVALID;
    allocator* Allocator = nullptr;
    hash_slot* Slots = nullptr;
    key*       Keys = nullptr;
    u32*       ItemSlots = nullptr;
    u32        SlotCapacity = 0;
    u32        ItemCapacity = 0;
    u32        Count = 0;

    inline hashset() { }
    inline hashset(allocator* _Allocator) : Allocator(_Allocator) {}

    void Add(const key& Key);
    bool Contains(const key& Key) const;
    void Remove(const key& Key);
    void Clear();
    void Release();

    inline const key* begin() const {
        return Keys;
    }

    inline const key* end() const {
        return Keys+Count;
    }
};

#endif