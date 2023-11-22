#ifndef ASYNC_WIN32_H
#define ASYNC_WIN32_H

struct mutex {
    CRITICAL_SECTION CriticalSection;

    inline void Init() {
        InitializeCriticalSection(&CriticalSection);
    }

    inline void Lock() {
        EnterCriticalSection(&CriticalSection);
    }

    inline void Unlock() {
        LeaveCriticalSection(&CriticalSection);
    }

    inline void Release() {
        DeleteCriticalSection(&CriticalSection);
    }
};

inline u64 Get_Current_Thread_ID() {
    return GetCurrentThreadId();
}

#endif