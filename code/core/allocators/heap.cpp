//NOTE(EVERYONE): Helper red black tree macros
#define HEAP_RED_NODE 0
#define HEAP_BLACK_NODE 1

#define Heap_Get_Parent(Node) ((heap_block_node*)((Node)->Color & ~HEAP_BLACK_NODE))
#define Heap_Get_Color(Node) ((Node)->Color & HEAP_BLACK_NODE)

#define Heap_Set_Red(Node)            ((Node)->Color &= (~(uptr)HEAP_BLACK_NODE))
#define Heap_Set_Black(Node)          ((Node)->Color |= ((uptr)HEAP_BLACK_NODE))
#define Heap_Set_Parent(Node, Parent) ((Node)->Color = Heap_Get_Color(Node) | (uptr)(Parent))
typedef struct heap_block heap_block;
typedef struct heap_block_node heap_block_node;

inline heap::heap() { }

inline heap::heap(allocator* _Allocator, uptr _MinimumBlockSize) : 
    Allocator(_Allocator), Arena(Allocator), MinimumBlockSize((uptr)Ceil_Pow2((u64)_MinimumBlockSize)) { } 

void* heap::Allocate_Internal(uptr Size) {
    //NOTE(EVERYONE): Allocating a block conceptually is a super simple process.
    //1. Find the best fitting block. 
    //2. Split it if the size requested is smaller than the block size
    //3. Get the pointer from the heap block
    
    if(!Size) return NULL;
    heap_block* Block = Find_Best_Fitting_Block(Size);
    if(!Block)
    {
        uptr BlockSize = LastBlockSize*2;

        if(BlockSize < Size) {
            BlockSize = (uptr)Ceil_Pow2((u64)Size);
        }

        LastBlockSize = BlockSize;

        if(MinimumBlockSize != (uptr)-1) {
            BlockSize = Max(BlockSize, MinimumBlockSize);
        }

        Block = Create_Memory_Block(BlockSize);
        if(!Block) return NULL;
    }
    
    Split_Block(Block, Size);
    
    //NOTE(EVERYONE): The block offset always starts at the beginning of the blocks metadata.
    //This is not where the returned memory starts. It always starts right after the metadata 
    //so we must offset an additional pointer size
    size_t Offset = Block->Offset + sizeof(heap_block*);
    Assert(Offset + Size <= Block->Block->Size, "Invalid block offset!");
    void* Result = Block->Block->Ptr + Offset;
    
#ifdef DEBUG_BUILD
    Add_To_Allocated_List(Block);
    Assert(Verify(), "Invalid heap!");
#endif
        
    return Result;
}

void heap::Free_Internal(void* Memory) {
    if(Memory)
    {
        //NOTE(EVERYTONE): Freeing a block is a little more complex
        //1. Get the block's metadata from the memory address. Validate that its the correct block
        //2. Check to see if the blockers neighboring blocks are free. And do the following:
        //    -If no neighboring blocks are free, add the block to the free block tree
        //    -If the left block is free, merge the left and current block, and delete the current block
        //    -If the right block is free, merge the current and right block, and delete the right block
        //    -If both are free, merge left block with the current and right block, and delete the current and right block
        
        heap_block** BlockPtr = (heap_block**)((heap_block**)Memory - 1);
        heap_block* Block = *BlockPtr;
        
        Assert((Block->Block->Ptr + Block->Offset + sizeof(heap_block*)) == Memory, "Invalid memory ptr!");
        
#ifdef DEBUG_BUILD
        Assert(Is_Block_Allocated(Block), "Block has already been freed!");
        Remove_Allocated_Block(Block);
#endif
        
        heap_block* LeftBlock  = Block->Prev;
        heap_block* RightBlock = Block->Next;
        
        bool IsLeftBlockFree  = Is_Block_Free(LeftBlock);
        bool IsRightBlockFree = Is_Block_Free(RightBlock);
        
        if(!IsLeftBlockFree && !IsRightBlockFree)
        {
            Add_Free_Block(Block);
        }
        else if(IsLeftBlockFree && !IsRightBlockFree)
        {
            Assert(LeftBlock->Block == Block->Block, "Invalid blocks");
            Assert((LeftBlock->Offset+LeftBlock->Size+sizeof(heap_block*)) == Block->Offset, "Invalid blocks");
            
            LeftBlock->Next = Block->Next;
            if(Block->Next) Block->Next->Prev = LeftBlock;
            
            Increase_Block_Size(LeftBlock, Block->Size+sizeof(heap_block*));
            Delete_Heap_Block(Block);
        }
        else if(!IsLeftBlockFree && IsRightBlockFree)
        {
            Assert(RightBlock->Block == Block->Block, "Invalid blocks");
            Assert((Block->Offset+Block->Size+sizeof(heap_block*)) == RightBlock->Offset, "Invalid blocks");
            
            Block->Next = RightBlock->Next;
            if(RightBlock->Next) RightBlock->Next->Prev = Block;
            
            Block->Size += RightBlock->Size+sizeof(heap_block*);
            Delete_Heap_Block(RightBlock);
            Add_Free_Block(Block);
        }
        else
        {
            Assert(LeftBlock->Block  == Block->Block, "Invalid blocks");
            Assert(RightBlock->Block == Block->Block, "Invalid blocks");
            Assert((LeftBlock->Offset+LeftBlock->Size+sizeof(heap_block*)) == Block->Offset, "Invalid blocks");
            Assert((Block->Offset+Block->Size+sizeof(heap_block*)) == RightBlock->Offset, "Invalid blocks");
            
            size_t Addend = Block->Size+RightBlock->Size+sizeof(heap_block*)*2;
            
            LeftBlock->Next = RightBlock->Next;
            if(RightBlock->Next) RightBlock->Next->Prev = LeftBlock;
            Increase_Block_Size(LeftBlock, Addend);
            Delete_Heap_Block(Block);
            Delete_Heap_Block(RightBlock);
        }
        
#ifdef DEBUG_BUILD
        Assert(Verify(), "Invalid heap!");
#endif
    }
}

void heap::Release() {

    heap_memory_block* MemoryBlock = FirstBlock;
    while(MemoryBlock) {
        heap_memory_block* BlockToDelete = MemoryBlock;
        MemoryBlock = MemoryBlock->Next;
        Allocator->Free(BlockToDelete);
    }

    FirstBlock = nullptr;
    Arena.Release();
    FreeBlockTree = {};
    OrphanBlocks = nullptr;
    LastBlockSize = 0;

#ifdef DEBUG_BUILD
    AllocatedList = nullptr;
#endif
}

#ifdef DEBUG_BUILD

#define Heap_Verify_Check(Condition) \
do \
{ \
if(!(Condition)) \
return false; \
} while(0)

bool heap::Verify() {
    bool OffsetVerified = Verify_Check_Offsets();
    if(!OffsetVerified) return false;
    
    heap_block_tree* Tree = &FreeBlockTree;
    if(Tree->Root) Heap_Verify_Check(Heap_Get_Color(Tree->Root) == HEAP_BLACK_NODE);
    
    uint32_t LeafBlackNodeCount = 0;
    if(Tree->Root)
    {
        for(heap_block_node* Node = Tree->Root; Node; Node = Node->LeftChild)
        {
            if(Heap_Get_Color(Node) == HEAP_BLACK_NODE)
                LeafBlackNodeCount++;
        }
    }
    
    bool TreeVerified = Tree_Verify(NULL, Tree->Root, 0, LeafBlackNodeCount);
    return TreeVerified;
}
#endif

//~NOTE(EVERYONE): Heap red-black tree implementation
inline bool heap::Is_Block_Free(heap_block* Block)
{
    return Block && Block->Node;
}

heap_block_node* heap::Get_Min_Block(heap_block_node* Node)
{
    if(!Node) return NULL;
    while(Node->LeftChild) Node = Node->LeftChild;
    return Node;
}

void heap::Swap_Block_Values(heap_block_node* NodeA, heap_block_node* NodeB)
{
    heap_block* Tmp = NodeB->Block;
    NodeB->Block    = NodeA->Block;
    NodeA->Block    = Tmp;
    
    NodeA->Block->Node = NodeA;
    NodeB->Block->Node = NodeB;
}

void heap::Rot_Tree_Left(heap_block_node* Node)
{
    heap_block_tree* Tree = &FreeBlockTree;

    heap_block_node* RightChild = Node->RightChild;
    if((Node->RightChild = RightChild->LeftChild) != NULL)
        Heap_Set_Parent(RightChild->LeftChild, Node);
    
    heap_block_node* Parent = Heap_Get_Parent(Node);
    Heap_Set_Parent(RightChild, Parent);
    *(Parent ? (Parent->LeftChild == Node ? &Parent->LeftChild : &Parent->RightChild) : &Tree->Root) = RightChild;
    RightChild->LeftChild = Node;
    Heap_Set_Parent(Node, RightChild);
}

void heap::Rot_Tree_Right(heap_block_node* Node)
{
    heap_block_tree* Tree = &FreeBlockTree;

    heap_block_node* LeftChild = Node->LeftChild;
    if((Node->LeftChild = LeftChild->RightChild) != NULL)
        Heap_Set_Parent(LeftChild->RightChild, Node);
    
    heap_block_node* Parent = Heap_Get_Parent(Node);
    Heap_Set_Parent(LeftChild, Parent);
    *(Parent ? (Parent->LeftChild == Node ? &Parent->LeftChild : &Parent->RightChild) : &Tree->Root) = LeftChild;
    LeftChild->RightChild = Node;
    Heap_Set_Parent(Node, LeftChild);
}

heap_block_node* heap::Create_Tree_Node()
{
    heap_block_tree* Tree = &FreeBlockTree;
    heap_block_node* Result = Tree->FreeNodes;
    if(Result) Tree->FreeNodes = Tree->FreeNodes->LeftChild;
    else Result = Arena.Push_Struct<heap_block_node>();
    Zero_Struct(Result);
    Assert((((uptr)Result) & 1) == 0, "Invalid alignment");
    return Result;
}

void heap::Delete_Tree_Node(heap_block_node* Node)
{
    Node->LeftChild = FreeBlockTree.FreeNodes;
    FreeBlockTree.FreeNodes = Node;
}

void heap::Fixup_Tree_Add(heap_block_node* Node)
{
    heap_block_tree* Tree = &FreeBlockTree;
    while(Node != Tree->Root && Heap_Get_Color(Heap_Get_Parent(Node)) == HEAP_RED_NODE)
    {
        if(Heap_Get_Parent(Node) == Heap_Get_Parent(Heap_Get_Parent(Node))->LeftChild)
        {
            heap_block_node* Temp = Heap_Get_Parent(Heap_Get_Parent(Node))->RightChild;
            if(Temp && (Heap_Get_Color(Temp) == HEAP_RED_NODE))
            {
                Heap_Set_Black(Temp);
                Node = Heap_Get_Parent(Node);
                Heap_Set_Black(Node);
                Node = Heap_Get_Parent(Node);
                Heap_Set_Red(Node);
            }
            else
            {
                if(Node == Heap_Get_Parent(Node)->RightChild)
                {
                    Node = Heap_Get_Parent(Node);
                    Rot_Tree_Left(Node);
                }
                
                Temp = Heap_Get_Parent(Node);
                Heap_Set_Black(Temp);
                Temp = Heap_Get_Parent(Temp);
                Heap_Set_Red(Temp);
                Rot_Tree_Right(Temp);
            }
        }
        else 
        {
            heap_block_node* Temp = Heap_Get_Parent(Heap_Get_Parent(Node))->LeftChild;
            if(Temp && (Heap_Get_Color(Temp) == HEAP_RED_NODE))
            {
                Heap_Set_Black(Temp);
                Node = Heap_Get_Parent(Node);
                Heap_Set_Black(Node);
                Node = Heap_Get_Parent(Node);
                Heap_Set_Red(Node);
            } 
            else 
            {
                if(Node == Heap_Get_Parent(Node)->LeftChild)
                {
                    Node = Heap_Get_Parent(Node);
                    Rot_Tree_Right(Node);
                }
                
                Temp = Heap_Get_Parent(Node);
                Heap_Set_Black(Temp);
                Temp = Heap_Get_Parent(Temp);
                Heap_Set_Red(Temp);
                Rot_Tree_Left(Temp);
            }
        }
    }
    
    Heap_Set_Black(Tree->Root);
}

void heap::Add_Free_Block(heap_block* Block)
{
    Assert(!Block->Node, "Block should be empty!");
    
    heap_block_tree* Tree = &FreeBlockTree;
    
    heap_block_node* Node = Tree->Root;
    heap_block_node* Parent = NULL;
    
    while(Node)
    {
        if(Block->Size <= Node->Block->Size)
        {
            Parent = Node;
            Node = Node->LeftChild;
        } 
        else
        {
            Parent = Node;
            Node = Node->RightChild;
        }
    }
    
    Node = Create_Tree_Node();
    Block->Node = Node;
    Node->Block = Block;
    
    Heap_Set_Parent(Node, Parent);
    if(Parent == NULL)
    {
        Tree->Root = Node;
        Heap_Set_Black(Node);
    }
    else
    {
        if(Block->Size <= Parent->Block->Size) Parent->LeftChild  = Node;
        else                                   Parent->RightChild = Node;
        Fixup_Tree_Add(Node);
    }
}

void heap::Fixup_Tree_Remove(heap_block_node* Node, heap_block_node* Parent, bool ChooseLeft)
{
    heap_block_tree* Tree = &FreeBlockTree;
    while(Node != Tree->Root && (Node == NULL || Heap_Get_Color(Node) == HEAP_BLACK_NODE))
    {
        if(ChooseLeft)
        {
            heap_block_node* Temp = Parent->RightChild;
            if(Heap_Get_Color(Temp) == HEAP_RED_NODE)
            {
                Heap_Set_Black(Temp);
                Heap_Set_Red(Parent);
                Rot_Tree_Left(Parent);
                Temp = Parent->RightChild;
            }
            
            if((Temp->LeftChild == NULL  || Heap_Get_Color(Temp->LeftChild)  == HEAP_BLACK_NODE) &&
               (Temp->RightChild == NULL || Heap_Get_Color(Temp->RightChild) == HEAP_BLACK_NODE))
            {
                Heap_Set_Red(Temp);
                Node = Parent;
                Parent = Heap_Get_Parent(Parent);
                ChooseLeft = Parent && (Parent->LeftChild == Node);
            }
            else
            {
                if(Temp->RightChild == NULL || Heap_Get_Color(Temp->RightChild) == HEAP_BLACK_NODE)
                {
                    Heap_Set_Black(Temp->LeftChild);
                    Heap_Set_Red(Temp);
                    Rot_Tree_Right(Temp);
                    Temp = Parent->RightChild;
                }
                
                (Heap_Get_Color(Parent) == HEAP_RED_NODE) ? Heap_Set_Red(Temp) : Heap_Set_Black(Temp);
                
                if(Temp->RightChild) Heap_Set_Black(Temp->RightChild);
                Heap_Set_Black(Parent);
                Rot_Tree_Left(Parent);
                break;
            }
        }
        else
        {
            heap_block_node* Temp = Parent->LeftChild;
            if(Heap_Get_Color(Temp) == HEAP_RED_NODE)
            {
                Heap_Set_Black(Temp);
                Heap_Set_Red(Parent);
                Rot_Tree_Right(Parent);
                Temp = Parent->LeftChild;
            }
            
            if((Temp->LeftChild  == NULL || Heap_Get_Color(Temp->LeftChild)  == HEAP_BLACK_NODE) &&
               (Temp->RightChild == NULL || Heap_Get_Color(Temp->RightChild) == HEAP_BLACK_NODE))
            {
                Heap_Set_Red(Temp);
                Node = Parent;
                Parent = Heap_Get_Parent(Parent);
                ChooseLeft = Parent && (Parent->LeftChild == Node);
            }
            else
            {
                if(Temp->LeftChild == NULL || Heap_Get_Color(Temp->LeftChild) == HEAP_BLACK_NODE)
                {
                    Heap_Set_Black(Temp->RightChild);
                    Heap_Set_Red(Temp);
                    Rot_Tree_Left(Temp);
                    Temp = Parent->LeftChild;
                }
                
                (Heap_Get_Color(Parent) == HEAP_RED_NODE) ? Heap_Set_Red(Temp) : Heap_Set_Black(Temp);
                
                if(Temp->LeftChild) Heap_Set_Black(Temp->LeftChild);
                Heap_Set_Black(Parent);
                Rot_Tree_Right(Parent);
                break;
            }
        }
    }
    
    if(Node) Heap_Set_Black(Node);
}

void heap::Remove_Free_Block(heap_block* Block)
{
    Assert(Block->Node, "Block should be allocated!");
    
    heap_block_tree* Tree = &FreeBlockTree;
    
    heap_block_node* Node = Block->Node;
    Assert(Node->Block == Block, "Invalid block node!");
    
    heap_block_node* Out = Node;
    
    if(Node->LeftChild && Node->RightChild)
    {
        Out = Get_Min_Block(Node->RightChild);
        Swap_Block_Values(Node, Out);
    }
    
    heap_block_node* ChildLink  = Out->LeftChild ? Out->LeftChild : Out->RightChild;
    heap_block_node* ParentLink = Heap_Get_Parent(Out);
    
    if(ChildLink) Heap_Set_Parent(ChildLink, ParentLink);
    
    bool ChooseLeft = ParentLink && ParentLink->LeftChild == Out;
    
    *(ParentLink ? (ParentLink->LeftChild == Out ? &ParentLink->LeftChild : &ParentLink->RightChild) : &Tree->Root) = ChildLink;
    
    if(Heap_Get_Color(Out) == HEAP_BLACK_NODE && FreeBlockTree.Root)
        Fixup_Tree_Remove(ChildLink, ParentLink, ChooseLeft);
    
    Delete_Tree_Node(Out);
    Block->Node = NULL;
}
//~End of heap red-black tree implementation

heap_block* heap::Create_Heap_Block()
{
    heap_block* Result = OrphanBlocks;
    if(Result) OrphanBlocks = OrphanBlocks->Next;
    else Result = Arena.Push_Struct<heap_block>();
    Zero_Struct(Result);
    return Result;
}

void heap::Delete_Heap_Block(heap_block* Block)
{
    if(Block->Node) Remove_Free_Block(Block);
    Block->Next = OrphanBlocks;
    OrphanBlocks = Block;
}

heap_block* heap::Find_Best_Fitting_Block(uptr Size)
{
    heap_block_node* Node   = FreeBlockTree.Root;
    heap_block_node* Result = NULL;
    while(Node)
    {
        int64_t Diff = (int64_t)Size - (int64_t)Node->Block->Size;
        if(!Diff) return Node->Block;
        
        if(Diff < 0)
        {
            Result = Node;
            Node = Node->LeftChild;
        }
        else
        {
            Node = Node->RightChild;
        }
    }
    
    if(Result) return Result->Block;
    return NULL;
}

void heap::Increase_Block_Size(heap_block* Block, uptr Addend)
{
    Remove_Free_Block(Block);
    Block->Size += Addend;
    Add_Free_Block(Block);
}

void heap::Split_Block(heap_block* Block, uptr Size)
{
    //NOTE(EVERYONE): When splitting a block, it is super important to check that the block has the requested size
    //plus a pointer size. We need to make sure that each new block has a pointer to its metadata before the actual
    //memory block starts. If the block is not large enough, we cannot split the block
    int64_t BlockDiff = (int64_t)Block->Size - (int64_t)(Size+sizeof(heap_block*));
    if(BlockDiff > 0)
    {
        uptr Offset = Block->Offset + sizeof(heap_block*) + Size;
        
        //NOTE(EVERYONE): If we can split the block, add the blocks metadata to the beginning of the blocks memory
        heap_block* NewBlock = Create_Heap_Block();
        heap_block** NewBlockPtr = (heap_block**)(Block->Block->Ptr + Offset);
        *NewBlockPtr = NewBlock;
        
        NewBlock->Block  = Block->Block;
        NewBlock->Size   = (uptr)BlockDiff;
        NewBlock->Offset = Offset;
        NewBlock->Prev   = Block;
        NewBlock->Next   = Block->Next;
        if(NewBlock->Next) NewBlock->Next->Prev = NewBlock;
        Block->Next      = NewBlock;
        Block->Size = Size;
        
        Add_Free_Block(NewBlock);
    }
    
    Remove_Free_Block(Block);
}

heap_block* heap::Add_Free_Block_From_Memory_Block(heap_memory_block* MemoryBlock)
{
#ifdef DEBUG_BUILD
    Assert(!MemoryBlock->FirstBlock, "Memory block should not have an entry");
#endif
    heap_block* Block = Create_Heap_Block();
    
    heap_block** BlockPtr = (heap_block**)(MemoryBlock->Ptr);
    *BlockPtr = Block;
    
    //NOTE(EVERYONE): Similar to splitting a block. When a new memory block is created, the underlying heap block
    //needs to have its sized take into accout the heaps metadata. So we always shrink the block by a pointer size
    Block->Block  = MemoryBlock;
    Block->Size   = MemoryBlock->Size-sizeof(heap_block*);
    Add_Free_Block(Block);
    
#ifdef DEBUG_BUILD
    //NOTE(EVERYONE): To verify that the heap offsets are working properly. Each memory block contains
    //a pointer to its first heap block in the memory block. These will then get properly split when
    //allocations are requested
    MemoryBlock->FirstBlock = Block;
#endif
    
    return Block;
}

heap_block* heap::Create_Memory_Block(uptr BlockSize)
{
    //NOTE(EVERYONE): When we allocate a memory block, we need the block size, plus the memory block metadata, and
    //addtional pointer size so we can add an underlying heap block that is BlockSize
    heap_memory_block* MemoryBlock = (heap_memory_block*)Allocator->Allocate(BlockSize+sizeof(heap_memory_block)+sizeof(heap_block*));

    MemoryBlock->Ptr = (uint8_t*)(MemoryBlock+1);
    MemoryBlock->Size   = BlockSize+sizeof(heap_block*);
    
    MemoryBlock->Next = FirstBlock;
    FirstBlock  = MemoryBlock;
    
    return Add_Free_Block_From_Memory_Block(MemoryBlock);
}

#ifdef DEBUG_BUILD
void heap::Add_To_Allocated_List(heap_block* Block) {
    if(AllocatedList) AllocatedList->PrevAllocated = Block;
    Block->NextAllocated = AllocatedList;
    AllocatedList = Block;
}

bool heap::Is_Block_Allocated(heap_block* Block)
{
    for(heap_block* TargetBlock = AllocatedList; TargetBlock; TargetBlock = TargetBlock->NextAllocated)
        if(Block == TargetBlock) return true;
    return false;
}

void heap::Remove_Allocated_Block(heap_block* Block) {
    if(Block->PrevAllocated) Block->PrevAllocated->NextAllocated = Block->NextAllocated;
    if(Block->NextAllocated) Block->NextAllocated->PrevAllocated = Block->PrevAllocated;
    if(Block == AllocatedList) AllocatedList = AllocatedList->NextAllocated;
    Block->PrevAllocated = Block->NextAllocated = NULL;
}

bool heap::Tree_Verify(heap_block_node* Parent, heap_block_node* Node, uint32_t BlackNodeCount, uint32_t LeafBlackNodeCount)
{
    heap_block_tree* Tree = &FreeBlockTree;
    if(Parent == NULL)
    {
        Heap_Verify_Check(Tree->Root == Node);
        Heap_Verify_Check(Node == NULL || Heap_Get_Color(Node) == HEAP_BLACK_NODE);
    }
    else
    {
        Heap_Verify_Check(Parent->LeftChild == Node || Parent->RightChild == Node);
    }
    
    if(Node)
    {
        Heap_Verify_Check(Heap_Get_Parent(Node) == Parent);
        if(Parent)
        {
            if(Parent->LeftChild == Node)
                Heap_Verify_Check(Parent->Block->Size >= Node->Block->Size);
            else
            {
                Heap_Verify_Check(Parent->RightChild == Node);
                Heap_Verify_Check(Parent->Block->Size <= Node->Block->Size);
            }
        }
        
        if(Heap_Get_Color(Node) == HEAP_RED_NODE)
        {
            if(Node->LeftChild)  Heap_Verify_Check(Heap_Get_Color(Node->LeftChild)  == HEAP_BLACK_NODE);
            if(Node->RightChild) Heap_Verify_Check(Heap_Get_Color(Node->RightChild) == HEAP_BLACK_NODE);
        }
        else
        {
            BlackNodeCount++;
        }
        
        if(!Node->LeftChild && !Node->RightChild)
            Heap_Verify_Check(BlackNodeCount == LeafBlackNodeCount);
        
        bool LeftResult  = Tree_Verify(Node, Node->LeftChild,  BlackNodeCount, LeafBlackNodeCount);
        bool RightResult = Tree_Verify(Node, Node->RightChild, BlackNodeCount, LeafBlackNodeCount);
        return LeftResult && RightResult;
    }
    
    return true;
}

bool heap::Verify_Check_Offsets()
{
    for(heap_memory_block* MemoryBlock = FirstBlock; MemoryBlock; MemoryBlock = MemoryBlock->Next)
    {
        heap_block* Block = MemoryBlock->FirstBlock;
        if(Block->Prev) { return false; }
        
        size_t Offset = 0;
        while(Block)
        {
            if(Block->Offset != Offset) { return false; }
            Offset += (sizeof(heap_block*)+Block->Size);
            Block = Block->Next;
            if(!Block)  if(Offset != MemoryBlock->Size) { return false; }
        }
    }
    
    return true;
}
#endif