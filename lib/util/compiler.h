/* compiler.h - Compiler-specific tricks.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_UTIL_COMPILER_H
#define WEAK_COMPILER_UTIL_COMPILER_H

#if defined(__GNUC__) || defined(__llvm__) || defined(__INTEL_COMPILER)
# define __weak_gnu_exts 1
#endif

#if defined(__weak_gnu_exts)
# define __weak_likely(x)       __builtin_expect(!!(x), 1)
# define __weak_unlikely(x)     __builtin_expect(!!(x), 0)
# define __weak_wur             __attribute__ ((warn_unused_result))
# define __weak_noinline        __attribute__ ((noinline))
# define __weak_really_inline  inline \
                                __attribute__ ((always_inline))
# define __weak_unused          __attribute__ ((unused))
#else
# define __weak_likely(x)
# define __weak_unlikely(x)
# define __weak_wur
# define __weak_noinline
# define __weak_really_inline
# define __weak_unused
#endif

#define __weak_to_string(x) #x

#ifdef USE_LOG
# define __weak_debug(block) block
#else
# define __weak_debug(block)
#endif /* USE_LOG */

#ifdef USE_LOG
# define __weak_debug_msg(fmt, ...) do { \
    printf("%s@%d:    " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
 } while (0);
#else
# define __weak_debug_msg(fmt, ...)
#endif /* USE_LOG */


#define __weak_array_size(x) (sizeof (x) / sizeof (*x))

#endif // WEAK_COMPILER_UTIL_COMPILER_H