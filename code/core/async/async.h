#ifndef ASYNC_H
#define ASYNC_H

// #if defined(COMPILER_MSVC)
// #include "async_msvc.h"
// #elif defined(COMPILER_GCC) && (defined(CPU_X86) || defined(CPU_X64))
// #include "async_gcc_x86_x64.h"
// #elif defined(COMPILER_GCC) && (defined(CPU_ARM) || defined(CPU_ARM64))
// #include "async_gcc_arm.h"
// #else
// #error "Not Implemented"
// #endif

#if defined(COMPILER_MSVC)
#include "async_msvc.h"
#elif defined(COMPILER_GCC) && (defined(CPU_X86) || defined(CPU_X64))
#elif defined(COMPILER_GCC) && defined(CPU_ARM)

#elif defined(COMPILER_GCC) && defined(CPU_AARCH64)

#else
#error Not Implemented
#endif

#if defined(OS_WIN32)
#include "async_win32.h"
#elif defined(OS_POSIX)
#include "async_posix.h"
#else
#error Not Implemented
#endif

#endif