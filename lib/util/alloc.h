/* alloc.h - Safe memory allocation functions.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_UTIL_ALLOC_H
#define FCC_UTIL_ALLOC_H

#include "util/compiler.h"
#include "util/unreachable.h"
#include <stddef.h>

#if __has_attribute(returns_nonnull) && __has_attribute(warn_unused_result) && __has_attribute(alloc_size)
#define __fcc_malloc  __attribute__((returns_nonnull, warn_unused_result, malloc, alloc_size(1)))
#define __fcc_calloc  __attribute__((returns_nonnull, warn_unused_result, malloc, alloc_size(1, 2)))
#define __fcc_realloc __attribute__((returns_nonnull, warn_unused_result, alloc_size(2)))
#else
#define __fcc_malloc
#define __fcc_calloc
#define __fcc_realloc
#endif

__fcc_malloc  wur void *fcc_malloc(size_t size);
__fcc_calloc  wur void *fcc_calloc(size_t nmemb, size_t size);
__fcc_realloc wur void *fcc_realloc(void *addr, size_t size);

#define fcc_new(type) fcc_calloc(1, sizeof (type))

/** Used to reduce #include <stdlib.h> bloat. */
void fcc_free(void *addr);

#endif // FCC_UTIL_ALLOC_H