#ifndef ASYNC_POSIX_H
#define ASYNC_POSIX_H

struct mutex {
    pthread_mutex_t Mutex;

    inline void Init() {
        pthread_mutexattr_t Attribute;
        pthread_mutexattr_init(&Attribute);
#if defined(PTHREAD_MUTEX_RECURSIVE) || defined(__FreeBSD__)
        pthread_mutexattr_settype(&Attribute, PTHREAD_MUTEX_RECURSIVE);
#else
        pthread_mutexattr_settype(&Attribute, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
        pthread_mutex_init(&Mutex, &Attribute);
    }

    inline void Lock() {
        pthread_mutex_lock(&Mutex);
    }

    inline void Unlock() {
        pthread_mutex_unlock(&Mutex);
    }

    inline void Release() {
        pthread_mutex_destroy(&Mutex);
    }
};

inline u64 Get_Current_Thread_ID() {
        // FIXME: On Linux, would the kernel task ID be more useful for debugging?
        // If so, detect NPTL at compile time and create TID_NPTL.h which uses gettid() instead.
#ifdef KERNEL_FREEBSD
		return pthread_getthreadid_np();
#elif defined(OS_MINGW)
        return (u64)pthread_self().p;
#else
        return (u64)pthread_self();
#endif
}

#endif