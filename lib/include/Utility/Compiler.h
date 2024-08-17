/* Compiler.h - Compiler macros.
 * Copyright (C) 2024 epoll-reactor-2 <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_UTILITY_COMPILER_H
#define WEAK_COMPILER_UTILITY_COMPILER_H

#define WEAK_PRAGMA(P) _Pragma(#P)
#define WEAK_PRAGMA_PUSH _Pragma("GCC diagnostic push")
#define WEAK_PRAGMA_POP _Pragma("GCC diagnostic pop")
#define WEAK_PRAGMA_IGNORE(WARNING) WEAK_PRAGMA(GCC diagnostic ignored #WARNING)

#endif // WEAK_COMPILER_UTILITY_COMPILER_H