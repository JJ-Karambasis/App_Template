#ifndef HEAP_H
#define HEAP_H

struct heap_block;
struct heap_block_node;

struct heap_memory_block : buffer {
    heap_memory_block* Next;
#ifdef DEBUG_BUILD
    heap_block* FirstBlock;
#endif
};

struct heap_block {
    heap_memory_block* Block;
    uptr               Offset;
    uptr               Size;
    heap_block_node*   Node;
    heap_block*        Prev;
    heap_block*        Next;

#ifdef DEBUG_BUILD
    heap_block* NextAllocated;
    heap_block* PrevAllocated;
#endif
};

struct heap_block_node {
    heap_block*      Block;
    uptr             Color;
    heap_block_node* LeftChild;
    heap_block_node* RightChild;
};

struct heap_block_tree {
    heap_block_node* Root = nullptr;
    heap_block_node* FreeNodes = nullptr;
};

struct heap : public allocator {
    allocator*         Allocator = nullptr;
    arena              Arena;
    heap_memory_block* FirstBlock = nullptr;
    heap_block_tree    FreeBlockTree;
    heap_block*        OrphanBlocks = nullptr;
    uptr               MinimumBlockSize = (uptr)-1;
    uptr               LastBlockSize = 0;

#ifdef DEBUG_BUILD
    heap_block* AllocatedList = nullptr;
#endif

    heap();
    heap(allocator* Allocator, uptr MinimumBlockSize = (uptr)-1);

    void  Release();
    
protected:
    void* Allocate_Internal(uptr Size) override;
    void  Free_Internal(void* Memory) override;
public:

#ifdef DEBUG_BUILD
    bool Verify();
#endif

private:
    bool Is_Block_Free(heap_block*);
    heap_block_node* Get_Min_Block(heap_block_node*);
    void Swap_Block_Values(heap_block_node*, heap_block_node*);
    void Rot_Tree_Left(heap_block_node*);
    void Rot_Tree_Right(heap_block_node*);
    heap_block_node* Create_Tree_Node();
    void Delete_Tree_Node(heap_block_node*);
    void Fixup_Tree_Add(heap_block_node*);
    void Fixup_Tree_Remove(heap_block_node*, heap_block_node*, bool);
    void Add_Free_Block(heap_block*);
    void Remove_Free_Block(heap_block*);
    heap_block* Create_Heap_Block();
    void Delete_Heap_Block(heap_block*);
    heap_block* Find_Best_Fitting_Block(uptr);
    void Increase_Block_Size(heap_block*, uptr);
    void Split_Block(heap_block*, uptr);
    heap_block* Add_Free_Block_From_Memory_Block(heap_memory_block*);
    heap_block* Create_Memory_Block(uptr);

#ifdef DEBUG_BUILD
    void Add_To_Allocated_List(heap_block*);
    bool Is_Block_Allocated(heap_block*);
    void Remove_Allocated_Block(heap_block*);

    bool Tree_Verify(heap_block_node*, heap_block_node*, u32, u32);
    bool Verify_Check_Offsets();
#endif

};

#endif