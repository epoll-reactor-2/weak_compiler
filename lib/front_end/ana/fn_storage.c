/* fn_storage.c - Storage for function declarations.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/ana/fn_storage.h"
#include "front_end/ast/ast.h"
#include "util/alloc.h"
#include "util/crc32.h"
#include <string.h>

void fn_storage_init(fn_storage_t *s)
{
    hashmap_reset(s, 512);
}

void fn_storage_free(fn_storage_t *s)
{
    hashmap_foreach(s, k, v) {
        (void) k;
        struct builtin_fn *fn = (struct builtin_fn *) v;
        weak_free(fn);
    }
    hashmap_destroy(s);
}

void fn_storage_push(
    fn_storage_t       *s,
    const char         *name,
    struct ast_fn_decl *decl
) {
    hashmap_put(s, crc32_string(name), (uint64_t) decl);
}

struct ast_fn_decl *fn_storage_lookup(
    fn_storage_t *s,
    const char   *name
) {
    uint64_t hash = crc32_string(name);
    bool     ok   = 0;
    uint64_t addr = hashmap_get(s, hash, &ok);

    if (!ok || addr == 0)
        return NULL;

    return (struct ast_fn_decl *) addr;
}