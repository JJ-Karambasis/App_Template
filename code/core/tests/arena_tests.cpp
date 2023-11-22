struct ArenaTest : public testing::Test {
protected:
    void SetUp() override {
        Allocator = tracking_allocator(allocator::Get_Default());
        Arena = arena(&Allocator);
    }

    void TearDown() override {
        Arena.Release();
        // ASSERT_EQ(Allocator.CurrentAllocated.Value, 0);
        // ASSERT_EQ(Allocator.TotalAllocated.Value, BaseAllocator.TotalFreed.Value);
    }

    tracking_allocator Allocator;
    arena Arena;
};

TEST_F(ArenaTest, Pushing) {
    void* Result = Arena.Push(3123, 1);
    ASSERT_NE(Result, nullptr);

    //Block is aligned to the next power of two of the first allocation. All blocks
    //are then doubled on resize
    ASSERT_EQ(Arena.FirstBlock, Arena.CurrentBlock);
    ASSERT_EQ(Arena.CurrentBlock->Size, 4096);
    ASSERT_EQ(Arena.CurrentBlock->Used, 3123);

    //Fill remaining block
    Arena.Push(Arena.CurrentBlock->Size-Arena.CurrentBlock->Used, 1);
    ASSERT_EQ(Arena.FirstBlock, Arena.CurrentBlock);
    ASSERT_EQ(Arena.CurrentBlock->Used, Arena.CurrentBlock->Size);

    Arena.Push(812, 1);
    ASSERT_NE(Arena.FirstBlock, Arena.CurrentBlock);
    ASSERT_EQ(Arena.FirstBlock->Next, Arena.CurrentBlock);
    ASSERT_EQ(Arena.CurrentBlock->Size, 8192);
    ASSERT_EQ(Arena.CurrentBlock->Used, 812);
}

TEST_F(ArenaTest, Clearing) {
    Arena.Push(4096, 1);
    Arena.Push(412, 1);
    Arena.Push(8000, 1);

    Arena.Clear();

    ASSERT_EQ(Arena.FirstBlock, Arena.CurrentBlock);
    ASSERT_EQ(Arena.FirstBlock->Next->Next, Arena.LastBlock);

    //Each block is a power of two from each other
    uptr Size = Arena.FirstBlock->Size;
    for(arena_block* Block = Arena.FirstBlock; Block; Block = Block->Next) {
        ASSERT_EQ(Block->Size, Size);
        ASSERT_EQ(Block->Used, 0);

        Size *= 2;
    }
}