TEST(HashmapTest, Hashmap) {
    tracking_allocator BaseAllocator(allocator::Get_Default());

    hashmap<int, double> TestMap;
    TestMap = hashmap<int, double>(&BaseAllocator);

    for(int i = 0; i < 32; i++) {
        TestMap.Add(i, i);
    }
    ASSERT_EQ(TestMap.Count, 32);

    for(int i = 0; i < 32; i++) {
        ASSERT_EQ(TestMap[i], i);
    }

    for(int i = 32; i < 64; i++) {
        TestMap.Add(i, i);
    }
    ASSERT_EQ(TestMap.Count, 64);

    for(int i = 0; i < 64; i++) {
        ASSERT_EQ(TestMap[i], i);
    }

    ASSERT_NE(TestMap.Get(31), nullptr);
    TestMap.Remove(31);
    ASSERT_EQ(TestMap.Get(31), nullptr);
    ASSERT_EQ(TestMap.Count, 63);

    TestMap.Clear();
    ASSERT_EQ(TestMap.Count, 0);

    TestMap.Release();

    // ASSERT_EQ(BaseAllocator.CurrentAllocated.Value == 0);
    // ASSERT_EQ(BaseAllocator.TotalAllocated.Value == BaseAllocator.TotalFreed.Value);
}