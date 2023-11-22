#if defined(OS_WIN32)
u64 Get_Performance_Counter() {
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return (u64)Result.QuadPart;
}

u64 Get_Performance_Frequency() {
    LARGE_INTEGER Result;
    QueryPerformanceFrequency(&Result);
    return (u64)Result.QuadPart;
}

void Delay(u32 Milliseconds) {
    Sleep(Milliseconds);
}
#elif defined(OS_POSIX)
static const u64 NS_PER_SECOND = 1000000000LL;

u64 Get_Performance_Counter() {
    timespec Now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &Now);

    u64 Result = Now.tv_sec;
    Result *= NS_PER_SECOND;
    Result += Now.tv_nsec;

    return Result;
}

u64 Get_Performance_Frequency() {
    return NS_PER_SECOND;
}

void Delay(u32 Milliseconds) {
    //units are in microseconds
    usleep(Milliseconds*1000);
}
#else
#error Not Implemented
#endif