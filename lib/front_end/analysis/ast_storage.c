/* ast_storage.с - Storage for declarations being AST nodes.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/analysis/ast_storage.h"
#include "util/alloc.h"
#include "util/crc32.h"
#include "util/hashmap.h"
#include <assert.h>
#include <string.h>

static uint16_t scope_depth;
static hashmap_t scopes;

void ast_storage_init_state()
{
    scope_depth = 0;
    hashmap_init(&scopes, 100);
}

void ast_storage_reset_state()
{
    scope_depth = 0;
    hashmap_foreach(&scopes, k, v) {
        (void) k;
        struct ast_storage_decl *decl = (struct ast_storage_decl *) v;
        weak_free(decl);
    }
    hashmap_destroy(&scopes);
}

void ast_storage_start_scope()
{
    ++scope_depth;
}

void ast_storage_end_scope()
{
    hashmap_foreach(&scopes, k, v) {
        struct ast_storage_decl *decl = (struct ast_storage_decl *) v;
        if (decl->depth == scope_depth)
            hashmap_remove(&scopes, k);
    }
    --scope_depth;
}

void ast_storage_push(const char *var_name, struct ast_node *ast)
{
    ast_storage_push_typed(var_name, D_T_UNKNOWN, /*indirection_lvl=*/0, ast);
}

void ast_storage_push_typed(
    const char      *var_name,
    enum data_type   dt,
    uint16_t         indirection_lvl,
    struct ast_node *ast
) {
    struct ast_storage_decl *decl = weak_calloc(1, sizeof (struct ast_storage_decl));
    decl->ast = ast;
    decl->data_type = dt;
    decl->name = var_name;
    decl->indirection_lvl = indirection_lvl;
    decl->read_uses = 0;
    decl->write_uses = 0;
    decl->depth = scope_depth;
    hashmap_put(&scopes, crc32_string(var_name), (uint64_t) decl);
}

struct ast_storage_decl *ast_storage_lookup(const char *var_name)
{
    uint64_t hash = crc32_string(var_name);
    bool ok       = 0;
    int64_t addr  = hashmap_get(&scopes, hash, &ok);

    if (!ok || addr == 0)
        return NULL;

    struct ast_storage_decl *decl = (struct ast_storage_decl *) addr;

    if (decl->depth > scope_depth)
        return NULL;

    return decl;
}

void ast_storage_add_read_use(const char *var_name)
{
    struct ast_storage_decl *decl = ast_storage_lookup(var_name);
    assert(decl && "Variable expected to be declared before");
    assert(decl->depth <= scope_depth && "Impossible case: variable depth > current depth");
    decl->read_uses++;
}

void ast_storage_add_write_use(const char *var_name)
{
    struct ast_storage_decl *decl = ast_storage_lookup(var_name);
    assert(decl && "Variable expected to be declared before");
    assert(decl->depth <= scope_depth && "Impossible case: variable depth > current depth");
    decl->write_uses++;
}

void ast_storage_current_scope_uses(ast_storage_decl_array_t *out_set)
{
    hashmap_foreach(&scopes, k, v) {
        (void) k;
        struct ast_storage_decl *decl = (struct ast_storage_decl *) v;
        if (decl->depth == scope_depth)
            vector_push_back(*out_set, decl);
    }
}
