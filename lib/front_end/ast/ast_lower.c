/* ast_lower.h - Remove of some abstractions on AST.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/ast/ast.h"
#include "util/alloc.h"
#include "util/hashmap.h"
#include "util/crc32.h"
#include "util/unreachable.h"
#include <assert.h>
#include <string.h>


#define DECL_NAME_MAX_LEN 256

/// \note: Functions cannot return array.
///        Function takes array as parameter via pointer.
///        Array can be declared as variable.
struct array_decl_info {
    char                 name[DECL_NAME_MAX_LEN];
    enum data_type       dt;
    struct ast_compound *arity;
    uint64_t             depth;
};

static uint64_t  scope_depth;
static hashmap_t storage;

static void storage_init()
{
    scope_depth = 0;
    hashmap_init(&storage, 100);
}

static void storage_reset()
{
    scope_depth = 0;
    hashmap_foreach(&storage, key, val) {
        (void) key;
        struct array_decl_info *decl = (struct array_decl_info *) val;
        weak_free(decl);
    }
    hashmap_destroy(&storage);
}

static void storage_start_scope()
{
    ++scope_depth;
}

static void storage_end_scope()
{
    hashmap_foreach(&storage, key, val) {
        struct array_decl_info *decl = (struct array_decl_info *) val;
        if (decl->depth == scope_depth)
            hashmap_remove(&storage, key);
    }
    --scope_depth;
}

/// \todo: Put information about each level of array arity.
static void storage_put(
    const char          *name,
    enum data_type       dt,
    struct ast_compound *arity
) {
    struct array_decl_info *decl = weak_calloc(1, sizeof (struct array_decl_info));

    strncpy(decl->name, name, DECL_NAME_MAX_LEN);
    decl->dt = dt;
    decl->arity = arity;
    decl->depth = scope_depth;
    hashmap_put(&storage, crc32_string(name), (uint64_t) decl);
}

static struct array_decl_info *storage_lookup(const char *name)
{
    uint64_t hash = crc32_string(name);
    bool     ok   = 0;
    int64_t  addr = hashmap_get(&storage, hash, &ok);

    if (!ok || addr == 0)
        weak_unreachable("Could not find variable `%s`.", name);

    struct array_decl_info *decl = (struct array_decl_info *) addr;

    if (decl->depth > scope_depth)
        return NULL;

    return decl;
}



static void visit_node(struct ast_node **ast);

static void visit_ast_array_decl(struct ast_node *ast)
{
    struct ast_array_decl *decl = ast->ast;

    storage_put(decl->name, decl->dt, decl->arity_list->ast);
}

static void visit_ast_compound(struct ast_node *ast)
{
    struct ast_compound *compound = ast->ast;

    for (uint64_t i = 0; i < compound->size; ++i)
        visit_node(&compound->stmts[i]);
}

static void visit_ast_function_decl(struct ast_node *ast)
{
    struct ast_function_decl *decl = ast->ast;

    storage_start_scope();

    struct ast_compound *args = decl->args->ast;
    if (args && args->size > 0)
        visit_node(&decl->args);

    storage_end_scope();

    visit_node(&decl->body);
}

/// Check if iterated array declaration "drops"
/// last arity level. For example:
///
///    int array[1][2][3];
///
///    for (int it[1][2] : array) {} /* Correct. */
///    for (int it[1] : array) {} /* Incorrect. */
///    for (int it[2][2] : array) {} /* Incorrect. */
static bool verify_iterated_array(
    struct ast_array_decl *iterated,
    struct ast_array_decl *target
) {
    struct ast_compound *iterated_list = iterated->arity_list->ast;
    struct ast_compound   *target_list =   target->arity_list->ast;

    if (iterated_list->size != target_list->size - 1)
        return 0;

    for (uint64_t i = 0; i < iterated_list->size; ++i) {
        assert(iterated_list->stmts[i]->type == AST_NUM);
        assert(  target_list->stmts[i]->type == AST_NUM);

        struct ast_num *iterated_idx = iterated_list->stmts[i]->ast;
        struct ast_num   *target_idx =   target_list->stmts[i]->ast;

        int32_t it_v = iterated_idx->value;
        int32_t ta_v =   target_idx->value;

        if (it_v != ta_v)
            return 0;
    }

    return 1;
}

/// 1. Determine size of array
/// 2. Make usual for header
/// 3. Make iterator being pointer
/// 4. Copy `for` body after iterator declaration
///
///     int main() {
///       int array[5];
///       int num = 0;
///       for (int *i : array) {
///         *i = ++num;
///       }
///       // ... //
///       for (int __i = 0; __i < /* size of */ array; ++__i) {
///         int *i = &array[__i];
///         *i = ++num;
///       }
///     }
/// ...
/// Or
/// ...
///     int main() {
///       for (int *i : f()) {
///         *i = 0;
///       }
///       // ... //
///       /* ??? */ *__range_target_1 = f();
///       for (int __i = 0; __i < /* size of */ __range_target_1; ++__i) {
///         int *i = &__range_target_1[__i];
///       }
///     }
/// ...
/// Or
/// ...
///     int main() {
///       int arr[1][2][3] = { ... };
///       for (int *ptr[1][2] : arr) {
///         for (int *i[1] : *ptr) {
///           for (int *j : *i) {
///             ++(*j);
///           }
///         }
///       }
///       // ... //
///       for (int __i1 = 0; __i1 < /* size of */ arr; ++__i1) {
///         int *ptr[1][2] = &arr[__i1];
///         for (int __i2 = 0; __i2 < /* size of */ *ptr; ++__i2) {
///           int *i[1] = &ptr[__i2];
///           for (int __i3 = 0; __i3 < /* size of */ *i; ++__i3) {
///             int *j = &i[__i3];
///             ++(*j); /* Action on j. */
///           }
///         }
///       }
///     }
static void visit_ast_for_range(struct ast_node **ast)
{
    struct ast_for_range *for_range = (*ast)->ast;
    struct ast_node      *iter      = for_range->iter;
    struct ast_node      *target    = for_range->range_target;
    struct ast_node      *body      = for_range->body;

    (void) for_range;
    (void) iter;
    (void) target;
    (void) body;

    weak_free((*ast)->ast);
}

static void visit_node(struct ast_node **ast)
{
    assert(ast);

    switch ((*ast)->type) {
    /// Only for full tree traversal.
    case AST_COMPOUND_STMT:
        visit_ast_compound(*ast);
        break;
    case AST_FUNCTION_DECL:
        visit_ast_function_decl(*ast);
        break;
    /// Stuff.
    case AST_ARRAY_DECL:
        visit_ast_array_decl(*ast);
        break;
    case AST_FOR_RANGE_STMT:
        visit_ast_for_range(ast);
        break;
    /// Ignore.
    default:
        break;
    }
}

void ast_lower(struct ast_node **ast)
{
    storage_init();
    visit_node(ast);
    storage_reset();
}