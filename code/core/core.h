#ifndef CORE_H
#define CORE_H

#include "platform_detect.h"

#if defined(ENVIRONMENT32)
#define PTR_SIZE 4
#elif defined(ENVIRONMENT64)
#define PTR_SIZE 8
#else
#error "Invalid bitness!"
#endif

#if defined(OS_WIN32) || defined(OS_MINGW)     // Win32 API
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <intrin.h>
    #undef WIN32_LEAN_AND_MEAN
    #undef NOMINMAX
#endif

#if defined(OS_POSIX)
#include <pthread.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
    #if defined(KERNEL_FREEBSD)
    #include <pthread_np.h>
    #endif
#endif

#if defined(OS_ANDROID)
#include <android/log.h>
#include <jni.h>
#include <android/native_activity.h>
#endif

#if defined(OS_OSX)
#include <CoreFoundation/CoreFoundation.h>

# ifdef __OBJC__
#  if !defined(OS_IOS)
#  import <AppKit/AppKit.h>
#  else
#  import <UIKit/UIKit.h>
#  endif
# endif

#endif

#if defined(COMPILER_MSVC)
#define shared_function __declspec(dllexport)
#else
#define shared_function
#endif

//Assertions are programmer only state checking macros. If the assertion fails, this will open the 
//debugger. If there is no debugger running this will crash the program.
#ifdef DEBUG_BUILD
# include <assert.h>
# define Assert(c, message) assert(c)
#else
# define Assert(c, message)
#endif

#define Null_Check(ptr) Assert((ptr) != nullptr, "Null pointer!")
#define Not_Implemented() Assert(false, "Not Implemented!")
#define Invalid_Default_Case() default: { Not_Implemented(); } break
#define Invalid_Code() Assert(false, "Invalid code!")

//Generic link list macros
#define SLL_Push_Back(First, Last, Node) (!First ? (First = Last = Node) : (Last->Next = Node, Last = Node))
#define SLL_Push_Front(First, Node) (Node->Next = First, First = Node)
#define SLL_Pop_Front(First) (First = First->Next)

//Some generic macros
#define Array_Count(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Clamp(min, v, max) Min(Max(min, v), max)
#define Abs(a) ((a) < 0 ? -(a) : (a))
#define Is_Pow2(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))
#define Sign_Of(a) ((a < 0) ? -1 : 1)
#define Reverse_Sign_Of(a) ((a < 0) ? 1 : -1)
#define Is_Positive_Sign(a) (Sign_Of(a) == 1)
#define Sq(v) ((v)*(v))
#define KB(x) ((x)*1024)
#define MB(x) (KB(x)*1024)
#define Align_Pow2(value, alignment) (((value) + (alignment)-1) & ~((alignment)-1))
#define _Stringify(x) #x
#define Stringify(x) _Stringify(x)


//Bitwise macros 
#define Set_Bit(x, bit_index) ((x) |= (1ul << (bit_index)))
#define Clear_Bit(x, bit_index) ((x) &= ~(1ul << (bit_index))) 
#define Get_Bit(x, bit_index) ((x) & (1ul << (bit_index)))

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <initializer_list>
#include <float.h>
#include <math.h>
#include <string.h>

#include "types.h"
#include "memory.h"
#include "common.h"
#include "timer.h"
#include "core_math.h"

#include "async/async.h"

#include "allocators/allocator.h"
#include "containers/containers.h"

#include "datetime.h"
#include "fileio.h"
#include "shared_library.h"
#include "thread_manager.h"
#include "logger.h"

struct core {
    allocator*      Allocator;
    thread_manager* ThreadManager;
    log_manager*    LogManager;

#ifdef OS_ANDROID
    static core* Create(ANativeActivity* Activity);
#else
    /// @brief: Creates the thread manager while initializing the main thread context
    /// @brief: Create the thread manager while initializing the main thread context
    /// @param Allocator: The allocator to allocate the thread manager memory. This must be a thread safe allocator
    /// @param UserData: Pointer to additional user data that can be configured with the thread.
    /// @return: The thread context of the newly created thread.
    static core* Create();
#endif

    static inline core* Get() {
        return s_Core;
    }

    static void Set(core* Core);

private:
    static core* s_Core;
};

#endif