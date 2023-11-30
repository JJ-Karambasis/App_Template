inline arena::arena() { }

inline arena::arena(allocator* _Allocator) : Allocator(_Allocator) { }

void* arena::Push(uptr Size, uptr Alignment, allocate_clear ClearFlag) {
    Assert(Is_Pow2(Alignment), "Invalid pow2");

    arena_block* Block = Get_Current_Block(Size, Alignment);
    if(!Block) {
        uptr BlockSize = 0;
        if(LastBlock) {
            //Increase the block size by two
            BlockSize = LastBlock->Size*2;
        }

        uptr Mask = Alignment-1;

        //If the block size still doesn't handle the allocation, round the block up to the nearest pow2
        if(BlockSize < (Size+Mask)) {
            BlockSize = (uptr)Ceil_Pow2((u64)(Size+Mask));
        }

        Block = Allocate_Block(BlockSize);
        SLL_Push_Back(FirstBlock, LastBlock, Block);
    }

    CurrentBlock = Block;
    
    CurrentBlock->Used = Align_Pow2(CurrentBlock->Used, Alignment);
    Assert(CurrentBlock->Used+Size <= CurrentBlock->Size, "Arena overflow!");

    void* Result = CurrentBlock->Ptr + CurrentBlock->Used;
    CurrentBlock->Used += Size;

    if(ClearFlag == allocate_clear::Clear) {
        Memory_Clear(Result, Size);
    }
    
    return Result;
}

arena_marker arena::Get_Marker() {
    return {
        .Arena = this,
        .CurrentBlock = CurrentBlock,
        .Marker = CurrentBlock ? CurrentBlock->Used : 0
    };
}

void arena::Set_Marker(arena_marker Marker) {
    Assert(this == Marker.Arena, "Invalid arena marker usage!");
    if(Marker.CurrentBlock) {
        CurrentBlock = Marker.CurrentBlock;
        CurrentBlock->Used = Marker.Marker;
        for(arena_block* Block = CurrentBlock->Next; Block; Block = Block->Next) {
            Block->Used = 0;
        }
    } else {
        CurrentBlock = FirstBlock;
        if(CurrentBlock) CurrentBlock->Used = 0;
    }
}

void arena::Clear() {
    for(arena_block* Block = FirstBlock; Block; Block = Block->Next) {
        Block->Used = 0;
    }
    CurrentBlock = FirstBlock;
}

void arena::Release() {
    allocator* TestAllocator = Allocator;
    arena_block* Block = FirstBlock;

    FirstBlock = nullptr;
    LastBlock = nullptr;
    CurrentBlock = nullptr;

    while(Block) {
        arena_block* BlockToDelete = Block;
        Block = BlockToDelete->Next;
        TestAllocator->Free(BlockToDelete);
    }
}

arena_block* arena::Get_Current_Block(uptr Size, uptr Alignment) {
    arena_block* Result = CurrentBlock;
    while(Result && (Result->Size < (Align_Pow2(Result->Used, Alignment)+Size))) {
        Result = Result->Next;
    }
    return Result;
}

arena_block* arena::Allocate_Block(uptr Size) {
    arena_block* Result = (arena_block*)Allocator->Allocate(Size+sizeof(arena_block));
    Result->Size = Size;
    Result->Ptr  = (u8*)(Result+1);
    Result->Used = 0;
    return Result;
}