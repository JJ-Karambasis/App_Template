#ifndef HASHMAP_H
#define HASHMAP_H

struct hash_slot {
    static const u32 INVALID = (u32)-1;

    u32 Hash = 0;
    u32 ItemIndex = (u32)-1;
    u32 BaseCount = 0;
};

template <typename type>
struct hasher {
    u32 Hash(const type& Value) const;
};

template <typename type>
struct comparer {
    bool Equal(const type& A, const type& B) const;
};

template <typename key, typename value>
struct kvp {
    const key& Key;
    value& Value;
};

template <typename key, typename value, typename hasher = hasher<key>, typename comparer = comparer<key>>
struct hashmap {
    static const u32 INVALID = hash_slot::INVALID;

    allocator* Allocator = nullptr;
    hash_slot* Slots = nullptr;
    key*       Keys = nullptr;
    value*     Values = nullptr;
    u32*       ItemSlots = nullptr;
    u32        SlotCapacity = 0;
    u32        ItemCapacity = 0;
    u32        Count = 0;

    inline hashmap() { }
    inline hashmap(allocator* _Allocator) : Allocator(_Allocator) {}

    void         Add(const key& Key, const value& Value);
    value*       Get(const key& Key);
    const value* Get(const key& Key) const;
    value&       operator[](const key& Key);
    const value& operator[](const key& Key) const;
    void         Remove(const key& Key);
    void         Clear();
    void         Release();

    struct kvp_iter {
        hashmap* Hashmap;
        u32      Index;

        inline kvp<key, value> operator*() const {
            return {Hashmap->Keys[Index], Hashmap->Values[Index]};
        }

        inline bool operator!=(const kvp_iter& Other) const {
            return Other.Index != Index;
        }

        inline kvp_iter& operator++() {
            Index++;
            return *this;
        } 
    };

    inline kvp_iter begin() {
        kvp_iter Result = {this, 0};
        return Result;
    }

    inline kvp_iter end() {
        kvp_iter Result = {this, Count};
        return Result;
    }
};


#endif