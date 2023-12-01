struct slot_map_thread_entry {
    async_handle<u32> Handle;
    u64               AllocateThreadID;
    u64               FreeThreadID;
    atom32            Allocated;
    bool              Freed;
};

TEST(AsyncSlotMapTests, AsyncSlotMap) {
    const u32 NumWorkerThreads = 64;
    const u32 Batch = 1024*64;

    arena Arena(allocator::Get_Default());

    async_slot_map<u32> SlotMap(&Arena, NumWorkerThreads*Batch);
    array<slot_map_thread_entry> Entries(&Arena, NumWorkerThreads*Batch);
    array<array<slot_map_thread_entry>> ThreadEntries(&Arena, NumWorkerThreads);

    array<thread_context*> AllocateThreads = array<thread_context*>(&Arena, NumWorkerThreads);
    array<thread_context*> FreeThreads = array<thread_context*>(&Arena, NumWorkerThreads);

    for(uptr i : Indices(ThreadEntries)) {
        ThreadEntries[i] = Entries.Subdivide(i*Batch, (i+1)*Batch);
    }

    for(uptr i : Indices(AllocateThreads)) {
        uptr* pIndex = Arena.Push_Struct<uptr>();
        *pIndex = i;

        FreeThreads[i] = thread_manager::Create_Thread([&](auto Context) {
            uptr Index = *(uptr*)Context->UserData;
            bool IsDone = false;
            while(!IsDone) {
                IsDone = true;
                for(slot_map_thread_entry& Entry : ThreadEntries[Index]) {
                    if(Atomic_Load(&Entry.Allocated) && !Entry.Freed) {

                        SlotMap.Free(Entry.Handle);

                        Entry.FreeThreadID = Get_Current_Thread_ID();
                        Entry.Freed = true;
                    }

                    if(!Entry.Freed) {
                        IsDone = false;
                    }
                }
            }

            return 0;
        }, pIndex);

        AllocateThreads[i] = thread_manager::Create_Thread([&](auto Context) {
            uptr Index = *(uptr*)Context->UserData;

            for(slot_map_thread_entry& Entry : ThreadEntries[Index]) {
                Entry.Handle = SlotMap.Allocate();
                Entry.AllocateThreadID = Get_Current_Thread_ID();

                EXPECT_FALSE(Entry.Handle.Is_Null());

                Atomic_Fence_Rel();

                Atomic_Store(&Entry.Allocated, true);
            }

            return 0;
        }, pIndex);
    }

    thread_manager::Wait_All();

    //We have to push to get free entries first, so we always have twice as many push counts
    ASSERT_EQ(SlotMap.FreeIndices.Stats.PushCount.Value, SlotMap.FreeIndices.Stats.PopCount.Value*2);
    ASSERT_EQ(SlotMap.Stats.AllocatedCount.Value, SlotMap.Stats.FreedCount.Value);
    ASSERT_EQ(SlotMap.Slots.Count, SlotMap.Stats.AllocatedCount.Value);

    Arena.Release();
}