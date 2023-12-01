#define HEAP_ALLOC(Size) Heap.Test_Allocate(Size); ASSERT_TRUE(Heap.Verify())
#define HEAP_FREE(Memory) Heap.Test_Free(Memory); ASSERT_TRUE(Heap.Verify())

struct test_heap : public heap {
    test_heap(allocator* Allocator, size_t Size = -1) : heap(Allocator, Size) { }

    void* Test_Allocate(size_t Size) {
        return Allocate_Internal(Size);
    }

    void Test_Free(void* Memory) {
        Free_Internal(Memory);
    }
};

TEST(HeapTest, Heap) {
    tracking_allocator BaseAllocator(allocator::Get_Default());

    test_heap Heap(&BaseAllocator, KB(512));

    void* Memory1 = Heap.Test_Allocate(KB(10));
    void* Memory2 = Heap.Test_Allocate(KB(11));
    void* Memory3 = Heap.Test_Allocate(KB(12));
    void* Memory4 = Heap.Test_Allocate(KB(13));
    void* Memory5 = Heap.Test_Allocate(KB(9));
    void* Memory6 = Heap.Test_Allocate(KB(8));
    void* Memory7 = Heap.Test_Allocate(KB(7));
    ASSERT_TRUE(Heap.Verify());
    
    heap_block_tree* Tree = &Heap.FreeBlockTree;
    
    Heap.Test_Free(Memory5);
    ASSERT_TRUE(Heap.Verify());  
    ASSERT_TRUE(Memory5 == Tree->Root->LeftChild->Block->Block->Ptr + Tree->Root->LeftChild->Block->Offset + sizeof(heap_block*));
    Heap.Test_Free(Memory3);
    ASSERT_TRUE(Heap.Verify());
    Heap.Test_Free(Memory4);
    ASSERT_TRUE(Heap.Verify());
    ASSERT_TRUE(Memory3 == Tree->Root->Block->Block->Ptr + Tree->Root->Block->Offset + sizeof(heap_block*));
    
    Heap.Test_Free(Memory1);
    ASSERT_TRUE(Heap.Verify());
    void* Memory8 = Heap.Test_Allocate(KB(30));
    ASSERT_TRUE(Heap.Verify());
    Heap.Test_Free(Memory8);
    ASSERT_TRUE(Heap.Verify());

    Heap.Release();

    ASSERT_EQ(BaseAllocator.CurrentAllocated, 0);
    ASSERT_EQ(BaseAllocator.TotalAllocated, BaseAllocator.TotalFreed);

    Heap = test_heap(&BaseAllocator);

    uint32_t Loops = 5;
    for(uint32_t k = 0; k < Loops; k++)
    {
        
        void* FirstBatchBlocks[500];
        for(uint32_t i = 0; i < 500; i++)
        {
            FirstBatchBlocks[i] = HEAP_ALLOC(Random_Between(KB(1), KB(2)));
        }
        
        void* SecondBatchBlocks[1000];
        for(uint32_t i = 0; i < 500; i++)
        {
            SecondBatchBlocks[(i*2)] = HEAP_ALLOC(Random_Between(KB(1), KB(2)));
            SecondBatchBlocks[(i*2)+1] = HEAP_ALLOC(Random_Between(KB(1), KB(2)));
            HEAP_FREE(FirstBatchBlocks[i]);
        }
        
        void* ThirdBatchBlocks[1000];
        for(uint32_t i = 0; i < 200; i++)
        {
            for(uint32_t j = 0; j < 5; j++)
            {
                ThirdBatchBlocks[i*5+j] = HEAP_ALLOC(Random_Between(KB(1), KB(2)));
            }
            
            for(uint32_t j = 0; j < 5; j++)
            {
                HEAP_FREE(SecondBatchBlocks[i*5+j]);
            }
        }
        
        for(uint32_t i = 0; i < 1000; i++)
        {
            HEAP_FREE(ThirdBatchBlocks[i]);
        }
    }

    Heap.Release();

    ASSERT_EQ(BaseAllocator.CurrentAllocated, 0);
    ASSERT_EQ(BaseAllocator.TotalAllocated, BaseAllocator.TotalFreed);
}