#ifndef ASYNC_H
#define ASYNC_H

#ifdef OS_WIN32
#include "async_win32.h"
#endif

#ifdef OS_POSIX
#include "async_posix.h"
#endif

struct temp_lock {
    mutex* Mutex;

    inline temp_lock(mutex* _Mutex) : Mutex(_Mutex) { 
        Mutex->Lock();
    }

    inline ~temp_lock() {
        Mutex->Unlock();
    }
};

#define Lock_Block(mutex) temp_lock __Temp_Lock__(mutex)

#endif