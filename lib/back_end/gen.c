/* gen.c - Code generator.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "back_end/gen.h"
#include "back_end/back_end.h"
#include "front_end/ast/ast.h"
#include "util/unreachable.h"

static void visit(struct ast_node *ast);

static void visit_char(struct ast_char *ast)
{
}

static void visit_int(struct ast_int *ast)
{
}

static void visit_float(struct ast_float *ast)
{
}

static void visit_string(struct ast_string *ast)
{
}

static void visit_bool(struct ast_bool *ast)
{
}

static void visit_sym(struct ast_sym *ast)
{
}

static void visit_var_decl(struct ast_var_decl *ast)
{
}

static void visit_array_decl(struct ast_array_decl *ast)
{
}

static void visit_struct_decl(struct ast_struct_decl *ast)
{
}

static void visit_break(struct ast_break *ast)
{
}

static void visit_continue(struct ast_continue *ast)
{
}

static void visit_binary(struct ast_binary *ast)
{
}

static void visit_unary(struct ast_unary *ast)
{
}

static void visit_array_access(struct ast_array_access *ast)
{
}

static void visit_member(struct ast_member *ast)
{
}

static void visit_if(struct ast_if *ast)
{
}

static void visit_for(struct ast_for *ast)
{
}

static void visit_while(struct ast_while *ast)
{
}

static void visit_do_while(struct ast_do_while *ast)
{
}

static void visit_ret(struct ast_ret *ast)
{
}

static void visit_compound(struct ast_compound *ast)
{
    for (uint64_t i = 0; i < ast->size; ++i) {
        struct ast_node *s = ast->stmts[i];
        visit(s);
    }
}

static void visit_fn_decl(struct ast_fn_decl *ast)
{
    back_end_emit_sym(ast->name, back_end_seek());

    back_end_native_prologue(4);
    back_end_native_ret();
    back_end_native_epilogue(4);
}

static void visit_fn_call(struct ast_fn_call *ast)
{
}

static void visit_cast(struct ast_cast *ast)
{
}


static void visit(struct ast_node *ast)
{
    void *ptr = ast->ast;

    switch (ast->type) {
    case AST_CHAR:            visit_char(ptr); break;
    case AST_INT:             visit_int(ptr); break;
    case AST_FLOAT:           visit_float(ptr); break;
    case AST_STRING:          visit_string(ptr); break;
    case AST_BOOL:            visit_bool(ptr); break;
    case AST_SYMBOL:          visit_sym(ptr); break;
    case AST_VAR_DECL:        visit_var_decl(ptr); break;
    case AST_ARRAY_DECL:      visit_array_decl(ptr); break;
    case AST_STRUCT_DECL:     visit_struct_decl(ptr); break;
    case AST_BREAK_STMT:      visit_break(ptr); break;
    case AST_CONTINUE_STMT:   visit_continue(ptr); break;
    case AST_BINARY:          visit_binary(ptr); break;
    case AST_PREFIX_UNARY:    visit_unary(ptr); break;
    case AST_POSTFIX_UNARY:   visit_unary(ptr); break;
    case AST_ARRAY_ACCESS:    visit_array_access(ptr); break;
    case AST_MEMBER:          visit_member(ptr); break;
    case AST_IF_STMT:         visit_if(ptr); break;
    case AST_FOR_STMT:        visit_for(ptr); break;
    case AST_WHILE_STMT:      visit_while(ptr); break;
    case AST_DO_WHILE_STMT:   visit_do_while(ptr); break;
    case AST_RETURN_STMT:     visit_ret(ptr); break;
    case AST_COMPOUND_STMT:   visit_compound(ptr); break;
    case AST_FUNCTION_DECL:   visit_fn_decl(ptr); break;
    case AST_FUNCTION_CALL:   visit_fn_call(ptr); break;
    case AST_IMPLICIT_CAST:   visit_cast(ptr); break;
    default:
        weak_unreachable("Wrong AST type (numeric: %d).", ast->type);
    }
}

void back_end_gen(struct ast_node *ast)
{
    visit(ast);
}