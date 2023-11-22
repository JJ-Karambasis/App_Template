#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#define MAX_SCRATCH_COUNT 32
#define MAX_THREAD_COUNT 256

/// @brief: Class that implements scratch memory
struct scratch : public temp_arena {
    u32 ScratchIndex = 0;

    /// @brief: Main constructor for the scratch struct
    /// @param Arena: The memory arena to allocate scratch memory out of (usually comes from thread context)
    /// @param _ScratchIndex: The scratch index in the thread context to validate against
    scratch(arena* Arena, u32 _ScratchIndex);

    /// @brief: Main destructor. This will cleanup the memory allocated during the scratch lifetime
    ~scratch();
};

struct thread_context;
using thread_callback = function<s32(thread_context*)>;

#ifdef OS_ANDROID
struct android_activity {
    JavaVM*        VM;
    AAssetManager* AssetManager;
    string         InternalDataPath;
};

struct android_thread_data {
    JNIEnv*           Env;
    android_activity* Activity;
};
#endif

/// @brief: The base class for thread contexts. Contains the scratch memory, thread id, and thread callback function.
/// For the main thread, no thread callback is used.
struct thread_context {
    arena           ScratchPool[MAX_SCRATCH_COUNT];
    u32             CurrentScratchIndex;
    u64             ThreadID;
    thread_callback Callback;
    void*           UserData;

    /// @brief: Waits for the thread owning the thread context to finish. If this is the main thread context, this does nothing.
    virtual void Wait()   = 0;

    /// @brief: Deletes the thread and all of its data
    virtual void Delete();

    /// @brief: Static method to retrieve the thread context for the calling thread
    /// @return: The thread context for the calling thread
    static thread_context* Get();
#ifdef OS_ANDROID
    static android_thread_data Get_Android_Data();
#endif
private:
    static thread_local thread_context* s_ThreadContext;
};

struct thread_manager {
    friend struct thread_context;

    mutex                         Mutex;
    allocator*                    Allocator;
    hashmap<u64, thread_context*> ThreadMap;
    thread_context*               MainThreadContext;
    
    /// @brief: Retrieves the static instance of the thread manager
    /// @return: The thread manager
    static inline thread_manager* Get() {
        return s_ThreadManager;
    }

    /// @brief: Sets the static instance of the thread manager. Useful for hot-reloading
    /// @param ThreadManager: The thread manager to make the static instance of
    static inline void Set(thread_manager* ThreadManager) {
        s_ThreadManager = ThreadManager;
    }

#ifdef OS_OSX
    static string Get_Bundle_Path();
#endif

#ifdef OS_ANDROID
    static thread_manager* Create(allocator* Allocator, JavaVM* JavaVM, JNIEnv* JavaENV, AAssetManager* AssetManager, const char* InternalDataPath);
#else
    /// @brief: Creates the thread manager while initializing the main thread context
    /// @brief: Create the thread manager while initializing the main thread context
    /// @param Allocator: The allocator to allocate the thread manager memory. This must be a thread safe allocator
    /// @param UserData: Pointer to additional user data that can be configured with the thread.
    /// @return: The thread context of the newly created thread.
    static thread_manager* Create(allocator* Allocator);
#endif
    
    /// @brief: Waits for all threads to finish
    static void Wait_All();

    /// @brief: Deletes the thread manager and all the internal resources
    static void Delete();

    /// @brief: Creates a new thread while executing the thread callback 
    /// @param Function: The thread callback for the thread to execute
    /// @return: The thread context for the newly created thread
    static thread_context* Create_Thread(const thread_callback& Function, void* UserData);
    
private:
    static thread_manager* s_ThreadManager;
    thread_context* Get_Thread_Context();
};

/// @brief: Retrieves some temporary scratch memory whose lifetime last the entire scratch scope
/// @return: The scratch memory to allocate out of
scratch Get_Scratch();

#endif