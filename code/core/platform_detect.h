#ifndef PLATFORM_DETECT_H
#define PLATFORM_DETECT_H

#if defined(_MSC_VER)
    // Win32
    #define OS_WIN32
    #define COMPILER_MSVC
    
    #if defined(_M_X64)
        #define CPU_X64
        #define ENVIRONMENT64
    #elif defined(_M_IX86)
        #define CPU_X86
        #define ENVIRONMENT32
    #else
        #error "Unrecognized platform!"
    #endif
#elif defined(__GNUC__)
    // GCC Compiler family
    #define COMPILER_GCC

    #if defined(__APPLE__)
        #define OS_OSX
        #define OS_POSIX
        #include <TargetConditionals.h>
        #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
            #define OS_IOS
        #endif
    #endif

    #if defined(__FreeBSD__)
        #define OS_POSIX
        #define KERNEL_FREEBSD
    #endif

    #if defined(__linux__)
        #define OS_POSIX
        #define KERNEL_LINUX
    #endif

    #if defined(__MACH__)
        #define KERNEL_MACH
    #endif

    #if defined(__MINGW32__) || defined(_MINGW64__)
        #define OS_MINGW
        #define OS_POSIX
    #endif

    #if defined(__ANDROID__ )
        #define OS_ANDROID
    #endif

    #if defined(__x86_64__)
        #define CPU_X64
        #define ENVIRONMENT64
    #elif defined(__i386__)
        #define CPU_X86
        #define ENVIRONMENT32
    #elif defined(__arm__)
        #define CPU_ARM
        #define ENVIRONMENT32
        #if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
            // ARMv7
            #define CPU_ARM_VERSION 7
        #elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
            // ARMv6
            #define CPU_ARM_VERSION 6
        #else
            // Could support earlier ARM versions at some point using compiler barriers and swp instruction
            #error "Unrecognized ARM CPU architecture version!"
        #endif
        #if defined(__thumb__)
            // Thumb instruction set mode
            #define CPU_ARM_THUMB 1
        #endif
    #elif defined(__aarch64__)
        #define CPU_AARCH64
        #define ENVIRONMENT64
        #define CPU_ARM_VERSION 8
    #elif defined(__powerpc__) || defined(__POWERPC__) || defined(__PPC__)
        #define CPU_POWERPC 1
        #if defined(__powerpc64__)
            #define ENVIRONMENT64 8
        #else
            #define ENVIRONMENT32 4 // 32-bit architecture
        #endif
    #else
        #error "Unrecognized target CPU!"
    #endif
#else
    #error "Unrecognized compiler!"
#endif

#endif