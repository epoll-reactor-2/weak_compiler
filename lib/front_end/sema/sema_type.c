/* sema_type.c - Adding type information to the AST.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/ast/ast.h"
#include "front_end/sema/sema.h"
#include "util/unreachable.h"
#include <assert.h>
#include <stdio.h>

static enum data_type fn_ret_type;
static enum data_type last_type;

/*
    What to cast?

    Types A, B.

    *) A + B -> A,   or
       A + B -> B    to a bigger type.

    *) A a
       B b

       a = b, or
       b = r

    *) A f() {
           return B -> A
       }

    *) void f(A) { / * ... * / }
       B b = / * ... * /
       f(b -> A)

    *) A    f()  { / * ... * / }
       B    g()  { / * ... * / }
       void h(A) { / * ... * / }

       h(g() -> (B -> A))

    *) A arr[...]
       arr[A -> B]
*/

/*
    Example

    return (int a) + (float b) + (char c)


     int res = \     <- Result should be stored in int,
                \       cast to int.
                 \
                  +     <- float contains more information, than int,
                 / \       cast to float.
                /   \
           (int a)   \
                      \
                      +     <- float is bigger than char,
                     / \       cast to float.
                    /   \
              (float b)  \
                          \
                       (char c)
*/

static void visit(struct ast_node **ast);



/**********************************************
 **             Type selection               **
 **********************************************/
static bool any_of(enum data_type l, enum data_type r, enum data_type which)
{
    return l == which || r == which;
}

static enum data_type type_select_equal_size(enum data_type l, enum data_type r)
{
    /* Cast to more precise type. */
    if (any_of(l, r, D_T_FLOAT))
        return D_T_FLOAT;

    /* Cast to wider type. */
    if (any_of(l, r, D_T_INT) && any_of(l, r, D_T_CHAR))
        return D_T_INT;

    /* Cast to wider type. */
    if (any_of(l, r, D_T_INT) && any_of(l, r, D_T_BOOL))
        return D_T_INT;

    weak_unreachable(
        "Unknown pair of types: %s and %s",
        data_type_to_string(l),
        data_type_to_string(r)
    );
}

static enum data_type type_select(enum data_type l, enum data_type r)
{
    if (data_type_size[l] ==
        data_type_size[r])
        return type_select_equal_size(l, r);

    if (data_type_size[l] >
        data_type_size[r])
        return l;
    else
        return r;
}

/**********************************************
 **           Typed expressions              **
 **********************************************/
static void visit_char  () { last_type = D_T_CHAR; }
static void visit_num   () { last_type = D_T_INT; }
static void visit_float () { last_type = D_T_FLOAT; }
static void visit_string() { last_type = D_T_STRING; }
static void visit_bool  () { last_type = D_T_BOOL; }

static void visit_bin(struct ast_node **ast)
{
    struct ast_binary *bin = (*ast)->ast;

    visit(&bin->lhs);
    enum data_type l = last_type;
    visit(&bin->rhs);
    enum data_type r = last_type;

    if (l == r)
        return;

    struct ast_node *old = *ast;

    *ast = ast_implicit_cast_init(
        type_select(l, r),
        old,
        old->line_no,
        old->col_no
    );
    /* TODO: In variable assignment, cast if needed. */
}

static void visit_return(struct ast_node **ast)
{
    struct ast_ret *ret = (*ast)->ast;
    if (!ret->op)
        return;

    visit(&ret->op);

    if (ret->op->type == AST_IMPLICIT_CAST) {
        struct ast_implicit_cast *cast = ret->op->ast;
        cast->to = fn_ret_type;
        return;
    }

    if (last_type != fn_ret_type) {
        ret->op = ast_implicit_cast_init(
            fn_ret_type, /* Maybe introduce such term as "compatible types" */
            ret->op,     /* and determine, can we or not do conversion.     */
            ret->op->line_no,
            ret->op->col_no
        );
    }
}

/**********************************************
 **          Tree traversal only             **
 **********************************************/
static void visit_compound(struct ast_node *ast)
{
    struct ast_compound *compound = ast->ast;

    for (uint64_t i = 0; i < compound->size; ++i)
        visit(&compound->stmts[i]);
}

static void visit_for(struct ast_node *ast)
{
    struct ast_for *stmt = ast->ast;

    if (stmt->init)
        visit(&stmt->init);

    if (stmt->condition)
        visit(&stmt->condition);

    if (stmt->increment)
        visit(&stmt->increment);

    visit(&stmt->body);
}

static void visit_for_range(struct ast_node *ast)
{
    struct ast_for_range *stmt = ast->ast;
    /* Maybe there is nothing to check in the
       header of this statement. */
    visit(&stmt->body);
}

static void visit_fn_decl(struct ast_node *ast)
{
    struct ast_fn_decl *decl = ast->ast;
    fn_ret_type = decl->data_type;

    struct ast_compound *args = decl->args->ast;
    if (args && args->size > 0)
        visit(&decl->args);

    visit(&decl->body);
}

static void visit_decl(struct ast_node *ast)
{
    struct ast_var_decl *decl = ast->ast;
    (void) decl;
    /* TODO: In variable declaration, cast if needed. */
}

static void visit(struct ast_node **ast)
{
    assert(ast);

    switch ((*ast)->type) {
    /* Literals. */
    case AST_CHAR_LITERAL:
        visit_char();
        break;
    case AST_INTEGER_LITERAL:
        visit_num();
        break;
    case AST_FLOATING_POINT_LITERAL:
        visit_float();
        break;
    case AST_STRING_LITERAL:
        visit_string();
        break;
    case AST_BOOLEAN_LITERAL:
        visit_bool();
        break;
    /* Only for full tree traversal. */
    case AST_COMPOUND_STMT:
        visit_compound(*ast);
        break;
    case AST_FUNCTION_DECL:
        visit_fn_decl(*ast);
        break;
    case AST_FOR_STMT:
        visit_for(*ast);
        break;
    case AST_FOR_RANGE_STMT:
        visit_for_range(*ast);
        break;
    /* Expressions. */
    case AST_BINARY:
        visit_bin(ast);
        break;
    case AST_VAR_DECL:
        visit_decl(*ast);
        break;
    case AST_RETURN_STMT:
        visit_return(ast);
        break;
    /* Ignore. */
    default:
        break;
    }
}

/**********************************************
 **               Driver code                **
 **********************************************/
void sema_type(struct ast_node **ast)
{
    visit(ast);
}