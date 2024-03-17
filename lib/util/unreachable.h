/* unreachable.h - Missing return statement warning suppress.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_UTIL_UNREACHABLE_H
#define FCC_UTIL_UNREACHABLE_H

#include "util/compiler.h"
#include <stdio.h>

#define fcc_fatal_error(fmt, ...) do {       \
    printf("Fatal error ocurred at %s@%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    fflush(stdout);                           \
    __builtin_trap();                         \
} while (0);

#define fcc_fatal_errno(fmt, ...) do {       \
    printf("Fatal error ocurred at %s@%d: " fmt ": %s\n", __FILE__, __LINE__, ##__VA_ARGS__, strerror((errno))); \
    fflush(stdout);                           \
    __builtin_trap();                         \
} while (0);

#define fcc_unreachable(fmt, ...) do {       \
    printf("Unreachable point reached at %s@%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
    fflush(stdout);                           \
    __builtin_trap();                         \
} while (0);

#endif // FCC_UTIL_UNREACHABLE_H