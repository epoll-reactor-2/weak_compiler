/* gen.c - Code generator.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "back_end/gen.h"
#include "back_end/back_end.h"

/*** Temporary ***/
#include "back_end/risc_v.h"
/***           ***/

#include "front_end/ast/ast.h"
#include "util/compiler.h"
#include "util/unreachable.h"
#include <asm-generic/unistd.h>

/* Some LRU maybe? */
#define __tmp_reg_1 risc_v_reg_t0
#define __tmp_reg_2 risc_v_reg_t1
int     __tmp_reg = -1;

static void visit(struct ast_node *ast);

static void visit_float(unused struct ast_float *ast) {}
static void visit_string(unused struct ast_string *ast) {}
static void visit_bool(unused struct ast_bool *ast) {}
static void visit_sym(unused struct ast_sym *ast) {}
static void visit_var_decl(unused struct ast_var_decl *ast) {}
static void visit_array_decl(unused struct ast_array_decl *ast) {}
static void visit_struct_decl(unused struct ast_struct_decl *ast) {}
static void visit_break(unused struct ast_break *ast) {}
static void visit_continue(unused struct ast_continue *ast) {}
static void visit_unary(unused struct ast_unary *ast) {}
static void visit_array_access(unused struct ast_array_access *ast) {}
static void visit_member(unused struct ast_member *ast) {}
static void visit_if(unused struct ast_if *ast) {}
static void visit_for(unused struct ast_for *ast) {}
static void visit_while(unused struct ast_while *ast) {}
static void visit_do_while(unused struct ast_do_while *ast) {}
static void visit_fn_call(unused struct ast_fn_call *ast) {}
static void visit_cast(unused struct ast_implicit_cast *ast) {}

static void visit_char(unused struct ast_char *ast) {}

static void visit_int(struct ast_int *ast)
{
    back_end_native_li(__tmp_reg, ast->value);
}

static void visit_binary(struct ast_binary *ast)
{
    __tmp_reg = __tmp_reg_1;
    visit(ast->lhs);

    __tmp_reg = __tmp_reg_2;
    visit(ast->rhs);

    back_end_native_add(__tmp_reg, __tmp_reg_1, __tmp_reg_2);
}

static void visit_ret(struct ast_ret *ast)
{
    if (ast->op)
        visit(ast->op);
}

static void visit_compound(struct ast_compound *ast)
{
    for (uint64_t i = 0; i < ast->size; ++i) {
        struct ast_node *s = ast->stmts[i];
        visit(s);
    }
}

static void visit_fn_main(unused struct ast_fn_decl *ast)
{
    /* li    a7, __NR_exit
       li    a0, `return` result.
       ecall */
    back_end_native_li(risc_v_reg_a7, __NR_exit);
    back_end_native_li(risc_v_reg_a0, 42);
    back_end_native_syscall();
}

static void visit_fn_usual(unused struct ast_fn_decl *ast)
{
    /* TODO: Calculate how much variables are allocated
             in function.

             This codegen assumed to compute variable
             values using temporary registers and
             store value to variable via stack.

             Variable is also must be referred only
             by stack. */
    int stack_usage = 0;

    back_end_native_prologue(stack_usage);
    visit(ast->body);
    back_end_native_epilogue(stack_usage);
    back_end_native_ret();
}

/* _start must be located at the start address
   and perform jump to main.

   For now it contains only one instruction,
   but will be useful to make generic API. */
static uint64_t _start_size = 0x04;
/* This is setup before `main` code generation
   in order to jump from _start. */
static uint64_t main_seek   = 0x00;

static void visit_fn_decl(struct ast_fn_decl *ast)
{
    static bool main_emitted = 0;

    if (!strcmp(ast->name, "main")) {
        main_seek = back_end_seek() + _start_size;
        back_end_emit_sym(ast->name, main_seek);
        visit_fn_main(ast);

        uint64_t seek = back_end_seek() + _start_size;

        back_end_seek_set(0);
        back_end_native_call(main_seek);
        back_end_seek_set(seek);

        main_emitted = 1;
    } else {
        /* Where `main()` is generated, we don't need
           to play with additional _start function
           offset and it is correct without it now. */
        uint64_t off = main_emitted
            ? back_end_seek()
            : back_end_seek() + _start_size;

        back_end_emit_sym(ast->name, off);
        visit_fn_usual(ast);
    }
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
    back_end_emit_sym("_start", back_end_seek());

    visit(ast);
}