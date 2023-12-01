thread_local thread_context* thread_context::s_ThreadContext = nullptr;
thread_manager* thread_manager::s_ThreadManager = nullptr;

inline scratch::scratch(arena* Arena, u32 _ScratchIndex) : temp_arena(Arena), ScratchIndex(_ScratchIndex) { }

inline scratch::~scratch() {
    thread_context::Get()->CurrentScratchIndex--;
    Assert(thread_context::Get()->CurrentScratchIndex == ScratchIndex, "Scratch stack error!");
}

thread_context* thread_context::Get() {
    if(!s_ThreadContext) {
        s_ThreadContext = thread_manager::Get()->Get_Thread_Context();
    }

    return s_ThreadContext;
}

void thread_context::Delete() {
    for(arena& Scratch : ScratchPool) {
        Scratch.Release();
    }

    if(Callback.Invoker) {
        Callback.~function();
    }
}

#if defined(OS_WIN32)

struct win32_thread_context : public thread_context {
    HANDLE Thread;
    u32    Index = (u32)-1;
    u32    NextIndex = (u32)-1;

    virtual inline void Wait() override {
        if(Thread) {
            WaitForSingleObject(Thread, INFINITE);
        }
    }

    virtual void Delete();
};

struct win32_thread_manager : public thread_manager {
    win32_thread_context Threads[MAX_THREAD_COUNT];
    u32                  FirstFreeIndex = (u32)-1;
    u32                  MaxUsed = 0;

    win32_thread_context* Create_Thread_Context() {
        u32 Index;
        if(FirstFreeIndex != (u32)-1) {
            Index = FirstFreeIndex;
            FirstFreeIndex = Threads[Index].NextIndex;
        } else {
            Assert(MaxUsed < MAX_THREAD_COUNT, "Thread overflow!");
            Index = MaxUsed++;
        }

        win32_thread_context* ThreadContext = Threads + Index;
        ThreadContext->NextIndex = (u32)-1;
        ThreadContext->Index = Index;
        
        return ThreadContext;
    }

    void Delete_Thread_Context(win32_thread_context* Context) {
        Context->NextIndex = FirstFreeIndex;
        FirstFreeIndex = Context->Index;
        Context->Index = (u32)-1;
    }
};

void win32_thread_context::Delete() {
    if(Thread) {
        WaitForSingleObject(Thread, INFINITE);
        CloseHandle(Thread);
    }

    thread_context::Delete();


    win32_thread_manager* ThreadManager = (win32_thread_manager*)thread_manager::Get();
    ThreadManager->Delete_Thread_Context(this);
}

win32_thread_context* Create_Thread_Context_Raw(thread_manager* _ThreadManager, u64 ThreadID) {    
    win32_thread_manager* ThreadManager = (win32_thread_manager*)_ThreadManager;
    win32_thread_context* Result = ThreadManager->Create_Thread_Context();
    Result->ThreadID = ThreadID;
    ThreadManager->Mutex.Lock();
    ThreadManager->ThreadMap.Add(ThreadID, Result);
    ThreadManager->Mutex.Unlock();
    return Result;
}

thread_manager* thread_manager::Create(allocator* Allocator) {
    win32_thread_manager* ThreadManager = new(Allocator) win32_thread_manager;
    ThreadManager->Allocator = Allocator;
    ThreadManager->ThreadMap = hashmap<u64, thread_context*>(Allocator);
    ThreadManager->Mutex.Init();

    Set(ThreadManager);
    ThreadManager->MainThreadContext = Create_Thread_Context_Raw(ThreadManager, Get_Current_Thread_ID());
    return ThreadManager;
}

void thread_manager::Wait_All() {
    win32_thread_manager* ThreadManager = (win32_thread_manager*)Get();

    for(u32 ThreadIndex = 0; ThreadIndex < MAX_THREAD_COUNT; ThreadIndex++) {
        win32_thread_context* ThreadContext = ThreadManager->Threads + ThreadIndex;
        if(ThreadContext->Index != (u32)-1) {
            ThreadContext->Wait();
        }
    }
}

void thread_manager::Delete() {
    win32_thread_manager* ThreadManager = (win32_thread_manager*)Get();

    for(u32 ThreadIndex = 0; ThreadIndex < MAX_THREAD_COUNT; ThreadIndex++) {
        win32_thread_context* ThreadContext = ThreadManager->Threads + ThreadIndex;
        if(ThreadContext->Index != (u32)-1) {
            ThreadContext->Delete();            
        }
    }

    allocator* Allocator = ThreadManager->Allocator;
    ThreadManager->ThreadMap.Release();
    ThreadManager->Mutex.Release();
    operator delete(ThreadManager, Allocator);
}

static DWORD WINAPI Thread_Context_Callback(void* Parameter) {
    win32_thread_context* ThreadContext = (win32_thread_context*)Parameter;
    thread_manager* ThreadManager = thread_manager::Get();

    ThreadManager->Mutex.Lock();
    ThreadManager->ThreadMap.Add(Get_Current_Thread_ID(), ThreadContext);
    ThreadManager->Mutex.Unlock();

    s32 Result = ThreadContext->Callback(ThreadContext);

    ThreadManager->Mutex.Lock();
    ThreadManager->ThreadMap.Remove(Get_Current_Thread_ID());
    ThreadManager->Mutex.Unlock();

    return Result;
}

thread_context* thread_manager::Create_Thread(const thread_callback& Function, void* UserData) {
    win32_thread_manager* ThreadManager = (win32_thread_manager*)thread_manager::Get();

    win32_thread_context* Result = ThreadManager->Create_Thread_Context();
    Result->Callback = Function;
    Result->UserData = UserData;
    Result->Thread = CreateThread(nullptr, 0, Thread_Context_Callback, Result, 0, (DWORD*)&Result->ThreadID);
    return Result;
}

#elif defined(OS_ANDROID)

struct android_thread_context : public thread_context {
    pthread_t Thread;
    JNIEnv*   Env;
    u32       Index = (u32)-1;
    u32       NextIndex = (u32)-1;

    virtual inline void Wait() override {
        if(Thread) {
            pthread_join(Thread, nullptr);
        }
    }

    virtual void Delete() override;
};

struct android_thread_manager : public thread_manager {
    android_thread_context Threads[MAX_THREAD_COUNT];
    ANativeActivity*       Activity;
    u32                    FirstFreeIndex = (u32)-1;
    u32                    MaxUsed = 0;

    android_thread_context* Create_Thread_Context() {
        u32 Index;
        if(FirstFreeIndex != (u32)-1) {
            Index = FirstFreeIndex;
            FirstFreeIndex = Threads[Index].NextIndex;
        } else {
            Assert(MaxUsed < MAX_THREAD_COUNT, "Thread overflow!");
            Index = MaxUsed++;
        }

        android_thread_context* ThreadContext = Threads + Index;
        ThreadContext->NextIndex = (u32)-1;
        ThreadContext->Index = Index;
        
        return ThreadContext;
    }

    void Delete_Thread_Context(android_thread_context* Context) {
        Context->NextIndex = FirstFreeIndex;
        FirstFreeIndex = Context->Index;
        Context->Index = (u32)-1;
    }
};

void android_thread_context::Delete() {
    if(Thread) {
        pthread_join(Thread, nullptr);
    }
    thread_context::Delete();

    android_thread_manager* ThreadManager = (android_thread_manager*)thread_manager::Get();
    ThreadManager->Delete_Thread_Context(this);
}

android_thread_context* Create_Thread_Context_Raw(android_thread_manager* ThreadManager, u64 ThreadID, JNIEnv* JavaENV) {    
    android_thread_context* Result = ThreadManager->Create_Thread_Context();
    Result->ThreadID = ThreadID;
    Result->Env = JavaENV;
    ThreadManager->Mutex.Lock();
    ThreadManager->ThreadMap.Add(ThreadID, Result);
    ThreadManager->Mutex.Unlock();
    return Result;
}

thread_manager* thread_manager::Create(allocator* Allocator, ANativeActivity* Activity) {
    android_thread_manager* ThreadManager = new(Allocator) android_thread_manager;
    ThreadManager->Allocator = Allocator;
    ThreadManager->ThreadMap = hashmap<u64, thread_context*>(Allocator);
    ThreadManager->Activity = Activity;
    ThreadManager->Mutex.Init();

    Set(ThreadManager);
    ThreadManager->MainThreadContext = Create_Thread_Context_Raw(ThreadManager, Get_Current_Thread_ID(), Activity->env);
    return ThreadManager;
}

void thread_manager::Wait_All() {
    android_thread_manager* ThreadManager = (android_thread_manager*)Get();

    for(u32 ThreadIndex = 0; ThreadIndex < MAX_THREAD_COUNT; ThreadIndex++) {
        android_thread_context* ThreadContext = ThreadManager->Threads + ThreadIndex;
        if(ThreadContext->Index != (u32)-1) {
            ThreadContext->Wait();
        }
    }
}

void thread_manager::Delete() {
    android_thread_manager* ThreadManager = (android_thread_manager*)Get();

    for(u32 ThreadIndex = 0; ThreadIndex < MAX_THREAD_COUNT; ThreadIndex++) {
        android_thread_context* ThreadContext = ThreadManager->Threads + ThreadIndex;
        if(ThreadContext->Index != (u32)-1) {
            ThreadContext->Delete();            
        }
    }

    allocator* Allocator = ThreadManager->Allocator;
    ThreadManager->ThreadMap.Release();
    ThreadManager->Mutex.Release();
    operator delete(ThreadManager, Allocator);
}

static void* Thread_Context_Callback(void* Parameter) {
    android_thread_context* ThreadContext = (android_thread_context*)Parameter;
    android_thread_manager* ThreadManager = (android_thread_manager*)thread_manager::Get();

    JavaVM* JavaVM = ThreadManager->Activity->vm;
    JavaVM->AttachCurrentThread(&ThreadContext->Env, nullptr);

    ThreadManager->Mutex.Lock();
    ThreadManager->ThreadMap.Add(Get_Current_Thread_ID(), ThreadContext);
    ThreadManager->Mutex.Unlock();

    s32 Result = ThreadContext->Callback(ThreadContext);

    ThreadManager->Mutex.Lock();
    ThreadManager->ThreadMap.Remove(Get_Current_Thread_ID());
    ThreadManager->Mutex.Unlock();

    JavaVM->DetachCurrentThread();

    return nullptr;
}

thread_context* thread_manager::Create_Thread(const thread_callback& Function, void* UserData) {
    android_thread_manager* ThreadManager = (android_thread_manager*)thread_manager::Get();
    android_thread_context* Result = ThreadManager->Create_Thread_Context();
    Result->Callback = Function;
    Result->UserData = UserData;
    if(pthread_create(&Result->Thread, NULL, Thread_Context_Callback, Result) != 0)
        return nullptr;
    return Result;
}

#elif defined(OS_OSX)

struct osx_thread_context : public thread_context {
    pthread_t Thread;
    u32 Index = (u32)-1;
    u32 NextIndex = (u32)-1;

    virtual inline void Wait() override {
        if(Thread) {
            pthread_join(Thread, nullptr);
        }
    }

    virtual void Delete() override;
};

struct osx_thread_manager : public thread_manager {
    osx_thread_context Threads[MAX_THREAD_COUNT];
    string             BundlePath;
    u32                FirstFreeIndex = (u32)-1;
    u32                MaxUsed = 0;

    osx_thread_context* Create_Thread_Context() {
        u32 Index;
        if(FirstFreeIndex != (u32)-1) {
            Index = FirstFreeIndex;
            FirstFreeIndex = Threads[Index].NextIndex;
        } else {
            Assert(MaxUsed < MAX_THREAD_COUNT, "Thread overflow!");
            Index = MaxUsed++;
        }

        osx_thread_context* ThreadContext = Threads + Index;
        ThreadContext->NextIndex = (u32)-1;
        ThreadContext->Index = Index;
        
        return ThreadContext;
    }

    void Delete_Thread_Context(osx_thread_context* Context) {
        Context->NextIndex = FirstFreeIndex;
        FirstFreeIndex = Context->Index;
        Context->Index = (u32)-1;
    }
};

void osx_thread_context::Delete() {
    if(Thread) {
        pthread_join(Thread, nullptr);
    }
    thread_context::Delete();

    osx_thread_manager* ThreadManager = (osx_thread_manager*)thread_manager::Get();
    ThreadManager->Delete_Thread_Context(this);
}

osx_thread_context* Create_Thread_Context_Raw(thread_manager* _ThreadManager, u64 ThreadID) {    
    osx_thread_manager* ThreadManager = (osx_thread_manager*)_ThreadManager;
    osx_thread_context* Result = ThreadManager->Create_Thread_Context();
    Result->ThreadID = ThreadID;
    ThreadManager->Mutex.Lock();
    ThreadManager->ThreadMap.Add(ThreadID, Result);
    ThreadManager->Mutex.Unlock();
    return Result;
}

static string OSX_Get_Bundle_Path(allocator* Allocator) {
    // Get a reference to the main bundle
    CFBundleRef MainBundle = CFBundleGetMainBundle();
    if(MainBundle != nil) {
        // Get a reference to the file's URL
        CFURLRef ImageURL = CFBundleCopyBundleURL(MainBundle);

        // Convert the URL reference into a string reference
        CFStringRef ImagePath = CFURLCopyFileSystemPath(ImageURL, kCFURLPOSIXPathStyle);

        // Convert the string reference into a C string
        const char* Path = CFStringGetCStringPtr(ImagePath, kCFStringEncodingUTF8);
        
        //End with trailing slash since result is a path
        string Result = string::Concat(Allocator, Path, "/");

        CFRelease(ImagePath);
        CFRelease(ImageURL);

        return Result;
    }

    return {};
}

thread_manager* thread_manager::Create(allocator* Allocator) {
    osx_thread_manager* ThreadManager = new(Allocator) osx_thread_manager;
    ThreadManager->Allocator = Allocator;
    ThreadManager->ThreadMap = hashmap<u64, thread_context*>(Allocator);
    ThreadManager->Mutex.Init();

    ThreadManager->BundlePath = OSX_Get_Bundle_Path(Allocator);

    Set(ThreadManager);
    ThreadManager->MainThreadContext = Create_Thread_Context_Raw(ThreadManager, Get_Current_Thread_ID());
    return ThreadManager;
}


string thread_manager::Get_Bundle_Path() {
    osx_thread_manager* ThreadManager = (osx_thread_manager*)thread_manager::Get();
    return ThreadManager->BundlePath;
}

void thread_manager::Wait_All() {
    osx_thread_manager* ThreadManager = (osx_thread_manager*)Get();
    for(u32 ThreadIndex = 0; ThreadIndex < MAX_THREAD_COUNT; ThreadIndex++) {
        osx_thread_context* ThreadContext = ThreadManager->Threads + ThreadIndex;
        if(ThreadContext->Index != (u32)-1) {
            ThreadContext->Wait();
        }
    }
}

void thread_manager::Delete() {
    osx_thread_manager* ThreadManager = (osx_thread_manager*)Get();

    for(u32 ThreadIndex = 0; ThreadIndex < MAX_THREAD_COUNT; ThreadIndex++) {
        osx_thread_context* ThreadContext = ThreadManager->Threads + ThreadIndex;
        if(ThreadContext->Index != (u32)-1) {
            ThreadContext->Delete();            
        }
    }

    allocator* Allocator = ThreadManager->Allocator;
    ThreadManager->ThreadMap.Release();
    ThreadManager->Mutex.Release();
    operator delete(ThreadManager, Allocator);
}

static void* Thread_Context_Callback(void* Parameter) {
    osx_thread_context* ThreadContext = (osx_thread_context*)Parameter;
    osx_thread_manager* ThreadManager = (osx_thread_manager*)thread_manager::Get();

    ThreadManager->Mutex.Lock();
    ThreadManager->ThreadMap.Add(Get_Current_Thread_ID(), ThreadContext);
    ThreadManager->Mutex.Unlock();

    s32 Result = ThreadContext->Callback(ThreadContext);

    ThreadManager->Mutex.Lock();
    ThreadManager->ThreadMap.Remove(Get_Current_Thread_ID());
    ThreadManager->Mutex.Unlock();

    return nullptr;
}

thread_context* thread_manager::Create_Thread(const thread_callback& Function, void* UserData) {
    osx_thread_manager* ThreadManager = (osx_thread_manager*)thread_manager::Get();
    osx_thread_context* Result = ThreadManager->Create_Thread_Context();
    Result->Callback = Function;
    Result->UserData = UserData;
    if(pthread_create(&Result->Thread, NULL, Thread_Context_Callback, Result) != 0)
        return nullptr;
    return Result;
}

#else

# error Not Implemented
#endif

thread_context* thread_manager::Get_Thread_Context() {
    thread_context* Result = nullptr;

    u64 ThreadID = Get_Current_Thread_ID();

    {
        Lock_Block(&Mutex); 
        thread_context** pResult = ThreadMap.Get(ThreadID);
        if(pResult) {
            Result = *pResult;
        }
    }    

    if(!Result) {
#ifdef OS_ANDROID
        android_thread_manager* ThreadManager = (android_thread_manager*)thread_manager::Get();
        JNIEnv* Env;
        ThreadManager->Activity->vm->GetEnv((void**)&Env, JNI_VERSION_1_6);
        Result = Create_Thread_Context_Raw(ThreadManager, ThreadID, Env);
#else
        Result = Create_Thread_Context_Raw(thread_manager::Get(), ThreadID);
#endif
    }

    return Result;
}

scratch Get_Scratch() {
    thread_context* ThreadContext = thread_context::Get();

    Assert(ThreadContext->CurrentScratchIndex < Array_Count(ThreadContext->ScratchPool), "Scratch overflow!");
    u32 ScratchIndex = ThreadContext->CurrentScratchIndex++;
    arena* Arena = ThreadContext->ScratchPool + ScratchIndex;
    if(!Arena->Allocator) {
        *Arena = arena(thread_manager::Get()->Allocator);
    }

    return scratch(Arena, ScratchIndex);

}
