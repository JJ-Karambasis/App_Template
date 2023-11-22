#define DEFAULT_HASHMAP_SLOT_CAPACITY 64
#define DEFAULT_HASHMAP_ITEM_CAPACITY 32

inline u32 Expand_Slots(allocator* Allocator, hash_slot** Slots, u32 SlotCapacity, u32* ItemSlots) {
    u32 OldCapacity = SlotCapacity;
    SlotCapacity = SlotCapacity ? SlotCapacity*2 : DEFAULT_HASHMAP_SLOT_CAPACITY;
    u32 SlotMask = SlotCapacity-1;
    hash_slot* NewSlots = (hash_slot*)Allocator->Allocate(SlotCapacity*sizeof(hash_slot));
    
    for(u32 SlotIndex = 0; SlotIndex < SlotCapacity; SlotIndex++) {
        NewSlots[SlotIndex] = hash_slot();
    }
    
    for(u32 i = 0; i < OldCapacity; i++)
    {
        if((*Slots)[i].ItemIndex != hash_slot::INVALID)
        {
            u32 Hash = (*Slots)[i].Hash;
            u32 BaseSlot = (Hash & SlotMask); 
            u32 Slot = BaseSlot;
            while(NewSlots[Slot].ItemIndex != hash_slot::INVALID)
                Slot = (Slot + 1) & SlotMask;
            NewSlots[Slot].Hash = Hash;
            u32 ItemIndex = (*Slots)[i].ItemIndex;
            NewSlots[Slot].ItemIndex = ItemIndex;
            ItemSlots[ItemIndex] = Slot;
            NewSlots[BaseSlot].BaseCount++;
        }
    }

    if(*Slots) Allocator->Free(*Slots);
    *Slots = NewSlots;
    return SlotCapacity;
}


template <typename key, typename value>
inline u32 Expand_Items(allocator* Allocator, key** Keys, value** Values, u32** ItemSlots, u32 ItemCapacity) {
    u32 OldItemCapacity = ItemCapacity;
    ItemCapacity = ItemCapacity == 0 ? DEFAULT_HASHMAP_ITEM_CAPACITY : ItemCapacity*2;

    uptr ItemSize = sizeof(key)+sizeof(u32);
    if(Values) ItemSize += sizeof(value);

    key* NewKeys = (key*)Allocator->Allocate(ItemSize*ItemCapacity);
    value* NewValues = Values ? (value*)(NewKeys+ItemCapacity) : nullptr;
    u32* NewItemSlots = Values ? (u32*)(NewValues+ItemCapacity) : (u32*)(NewKeys+ItemCapacity);

    for(u32 ItemIndex = 0; ItemIndex < ItemCapacity; ItemIndex++)
        NewItemSlots[ItemIndex] = hash_slot::INVALID;

    if(*Keys) {
        Copy_Array(NewKeys, *Keys, OldItemCapacity);
        if(NewValues) Copy_Array(NewValues, *Values, OldItemCapacity);
        Copy_Array(NewItemSlots, *ItemSlots, OldItemCapacity);
        Allocator->Free(*Keys); 
    }

    *Keys = NewKeys;
    if(Values) *Values = NewValues;
    *ItemSlots = NewItemSlots;
    return ItemCapacity;
}

template <typename key, typename comparer>
inline u32 Find_Slot(hash_slot* Slots, u32 SlotCapacity, key* Keys, const key& Key, u32 Hash) {
    if(SlotCapacity == 0 || !Slots) return hash_slot::INVALID;

    u32 SlotMask = SlotCapacity-1;
    u32 BaseSlot = (Hash & SlotMask);
    u32 BaseCount = Slots[BaseSlot].BaseCount;
    u32 Slot = BaseSlot;
    
    while (BaseCount > 0) {
        if (Slots[Slot].ItemIndex != hash_slot::INVALID) {
            u32 SlotHash = Slots[Slot].Hash;
            u32 SlotBase = (SlotHash & SlotMask);
            if (SlotBase == BaseSlot) {
                Assert(BaseCount > 0, "");
                BaseCount--;
                            
                if (SlotHash == Hash) { 
                    comparer Comparer = {};
                    if(Comparer.Equal(Key, Keys[Slots[Slot].ItemIndex]))
                        return Slot;
                }
            }
        }
        
        Slot = (Slot + 1) & SlotMask;
    }
    
    return hash_slot::INVALID;
}

inline u32 Find_Free_Slot(hash_slot* Slots, u32 SlotMask, u32 BaseSlot) {
    u32 BaseCount = Slots[BaseSlot].BaseCount;
    u32 Slot = BaseSlot;
    u32 FirstFree = Slot;
    while (BaseCount) 
    {
        if (Slots[Slot].ItemIndex == hash_slot::INVALID && Slots[FirstFree].ItemIndex != hash_slot::INVALID) FirstFree = Slot;
        u32 SlotHash = Slots[Slot].Hash;
        u32 SlotBase = (SlotHash & SlotMask);
        if (SlotBase == BaseSlot) 
            --BaseCount;
        Slot = (Slot + 1) & SlotMask;
    }
    
    Slot = FirstFree;
    while (Slots[Slot].ItemIndex != hash_slot::INVALID) 
        Slot = (Slot + 1) & SlotMask;

    return Slot;
}

template <typename key, typename value, typename hasher, typename comparer>
inline void hashmap<key, value, hasher, comparer>::Add(const key& Key, const value& Value) {
    u32 Hash = hasher{}.Hash(Key);
    Assert((Find_Slot<key, comparer>(Slots, SlotCapacity, Keys, Key, Hash) == -1), "Entry already added!");
    
    if(Count >= (SlotCapacity - (SlotCapacity/3)))
        SlotCapacity = Expand_Slots(Allocator, &Slots, SlotCapacity, ItemSlots);

    u32 SlotMask = SlotCapacity-1;
    u32 BaseSlot = (Hash & SlotMask);
    u32 Slot = Find_Free_Slot(Slots, SlotMask, BaseSlot);
    
    if (Count >= ItemCapacity)
        ItemCapacity = Expand_Items(Allocator, &Keys, &Values, &ItemSlots, ItemCapacity);
    
    Assert(Count < ItemCapacity, "");
    Assert(Slots[Slot].ItemIndex == INVALID && (Hash & SlotMask) == BaseSlot, "");
    
    Slots[Slot].Hash = Hash;
    Slots[Slot].ItemIndex = Count;
    Slots[BaseSlot].BaseCount++;
    
    ItemSlots[Count] = Slot;
    Keys[Count] = Key;
    Values[Count] = Value;

    Count++;
}

template <typename key, typename value, typename hasher, typename comparer>
inline value* hashmap<key, value, hasher, comparer>::Get(const key& Key) {
    u32 Hash = hasher{}.Hash(Key);
    u32 Slot = Find_Slot<key, comparer>(Slots, SlotCapacity, Keys, Key, Hash);
    if(Slot == INVALID) return nullptr;
    return &Values[Slots[Slot].ItemIndex]; 
}

template <typename key, typename value, typename hasher, typename comparer>
const value* hashmap<key, value, hasher, comparer>::Get(const key& Key) const {
    u32 Hash = hasher{}.Hash(Key);
    u32 Slot = Find_Slot<key, comparer>(Slots, SlotCapacity, Keys, Key, Hash);
    if(Slot == INVALID) return nullptr;
    return &Values[Slots[Slot].ItemIndex]; 
}

template <typename key, typename value, typename hasher, typename comparer>
inline value& hashmap<key, value, hasher, comparer>::operator[](const key& Key) {
    u32 Hash = hasher{}.Hash(Key);
    u32 Slot = Find_Slot<key, comparer>(Slots, SlotCapacity, Keys, Key, Hash);
    Assert(Slot != INVALID, "Invalid hash slot");
    return Values[Slots[Slot].ItemIndex]; 
}

template <typename key, typename value, typename hasher, typename comparer>
inline const value& hashmap<key, value, hasher, comparer>::operator[](const key& Key) const {
    u32 Hash = hasher{}.Hash(Key);
    u32 Slot = Find_Slot<key, comparer>(Slots, SlotCapacity, Keys, Key, Hash);
    Assert(Slot != INVALID, "Invalid hash slot");
    return Values[Slots[Slot].ItemIndex]; 
}

template <typename key, typename value, typename hasher, typename comparer>
inline void hashmap<key, value, hasher, comparer>::Remove(const key& Key) {
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
        Values[Index] = Values[LastIndex];
        ItemSlots[Index] = ItemSlots[LastIndex];
        Slots[ItemSlots[LastIndex]].ItemIndex = Index;
    }

    Count--;
}

template <typename key, typename value, typename hasher, typename comparer>
inline void hashmap<key, value, hasher, comparer>::Clear() {
    for(u32 SlotIndex = 0; SlotIndex < SlotCapacity; SlotIndex++) {
        Slots[SlotIndex] = hash_slot();
    }

    Zero_Array(Keys, Count);
    Zero_Array(Values, Count);
    for(u32 ItemIndex = 0; ItemIndex < ItemCapacity; ItemIndex++) 
        ItemSlots[ItemIndex] = INVALID;
    Count = 0;
}

template <typename key, typename value, typename hasher, typename comparer>
inline void hashmap<key, value, hasher, comparer>::Release() {
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
        Values = nullptr;
        ItemSlots = nullptr;
    }
}