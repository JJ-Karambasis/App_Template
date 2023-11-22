template <typename key, typename hasher, typename comparer>
inline void hashset<key, hasher, comparer>::Add(const key& Key) {
    u32 Hash = hasher{}.Hash(Key);
    if(Find_Slot<key, comparer>(Slots, SlotCapacity, Keys, Key, Hash) == -1) {        
        if(Count >= (SlotCapacity - (SlotCapacity/3)))
            SlotCapacity = Expand_Slots(Allocator, &Slots, SlotCapacity, ItemSlots);

        u32 SlotMask = SlotCapacity-1;
        u32 BaseSlot = (Hash & SlotMask);
        u32 Slot = Find_Free_Slot(Slots, SlotMask, BaseSlot);
        
        if (Count >= ItemCapacity)
            ItemCapacity = Expand_Items(Allocator, &Keys, nullptr, &ItemSlots, ItemCapacity);
        
        Assert(Count < ItemCapacity, "");
        Assert(Slots[Slot].ItemIndex == INVALID && (Hash & SlotMask) == BaseSlot, "");
        
        Slots[Slot].Hash = Hash;
        Slots[Slot].ItemIndex = Count;
        Slots[BaseSlot].BaseCount++;
        
        ItemSlots[Count] = Slot;
        Keys[Count] = Key;

        Count++;
    }
}

template <typename key, typename hasher, typename comparer>
bool hashset<key, hasher, comparer>::Contains(const key& Key) const {
    u32 Hash = hasher{}.Hash(Key);
    u32 Slot = Find_Slot<key, comparer>(Slots, SlotCapacity, Keys, Key, Hash);
    return Slot != INVALID;
}

template <typename key, typename hasher, typename comparer>
inline void hashset<key, hasher, comparer>::Remove(const key& Key) {
    u32 Hash = hasher{}.Hash(Key);
    u32 Slot = Find_Slot<key, comparer>(Slots, SlotCapacity, Keys, Key, Hash);
    
    if(Slot == INVALID) return;

    u32 SlotMask = SlotCapacity-1;
    u32 BaseSlot = (Hash & SlotMask);
    u32 Index = Slots[Slot].ItemIndex;
    u32 LastIndex = Count-1;

    Slots[BaseSlot].BaseCount--;
    Slots[Slot].ItemIndex = INVALID;

    if(Index != LastIndex) {
        Keys[Index] = Keys[LastIndex];
        ItemSlots[Index] = ItemSlots[LastIndex];
        Slots[ItemSlots[LastIndex]].ItemIndex = Index;
    }

    Count--;
}

template <typename key, typename hasher, typename comparer>
inline void hashset<key, hasher, comparer>::Clear() {
    for(u32 SlotIndex = 0; SlotIndex < SlotCapacity; SlotIndex++) {
        Slots[SlotIndex] = hash_slot();
    }

    Zero_Array(Keys, Count);
    for(u32 ItemIndex = 0; ItemIndex < ItemCapacity; ItemIndex++) 
        ItemSlots[ItemIndex] = INVALID;
    Count = 0;
}

template <typename key, typename hasher, typename comparer>
inline void hashset<key, hasher, comparer>::Release() {
    SlotCapacity = 0;
    ItemCapacity = 0;
    Count = 0;
    if(Slots) {
        Allocator->Free(Slots);
        Slots = nullptr;
    }

    if(Keys) {
        Allocator->Free(Keys);
        Keys = nullptr;
        ItemSlots = nullptr;
    }
}