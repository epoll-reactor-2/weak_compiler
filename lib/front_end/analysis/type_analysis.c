/* type_analysis.h - Type checker.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/analysis/analysis.h"
#include "front_end/analysis/ast_storage.h"
#include "front_end/ast/ast.h"
#include "util/diagnostic.h"
#include "util/lexical.h"
#include "util/unreachable.h"
#include <assert.h>

static enum data_type last_dt = D_T_UNKNOWN;
static uint16_t       last_indir_lvl = 0;
static enum data_type last_return_dt = D_T_UNKNOWN;

static void reset_internal_state()
{
    last_dt = D_T_UNKNOWN;
    last_return_dt = D_T_UNKNOWN;
}

static void visit_ast_node(struct ast_node *ast);

static void visit_ast_char  () { last_indir_lvl = 0; last_dt = D_T_CHAR; }
static void visit_ast_num   () { last_indir_lvl = 0; last_dt = D_T_INT; }
static void visit_ast_float () { last_indir_lvl = 0; last_dt = D_T_FLOAT; }
static void visit_ast_string() { last_indir_lvl = 0; last_dt = D_T_STRING; }
static void visit_ast_bool  () { last_indir_lvl = 0; last_dt = D_T_BOOL; }

static bool correct_bin_ops(enum token_type op, enum data_type t)
{
    bool are_correct = false;

    switch (op) {
    case TOK_ASSIGN: /// Fall through.
        /// We need only check if there are same types on assignment.
        are_correct = true;
        break;
    /// Integer and floats.
    case TOK_PLUS:
    case TOK_MINUS:
    case TOK_STAR:
    case TOK_SLASH:
    case TOK_LE:
    case TOK_LT:
    case TOK_GE:
    case TOK_GT:
    case TOK_EQ:
    case TOK_NEQ:
    case TOK_OR:
    case TOK_AND:
    case TOK_MUL_ASSIGN:
    case TOK_DIV_ASSIGN:
    case TOK_PLUS_ASSIGN:
    case TOK_MINUS_ASSIGN: /// Fall through.
        are_correct |= t == D_T_INT;
        are_correct |= t == D_T_CHAR;
        are_correct |= t == D_T_BOOL;
        are_correct |= t == D_T_FLOAT;
        break;
    /// Only integers.
    case TOK_BIT_OR:
    case TOK_BIT_AND:
    case TOK_XOR:
    case TOK_SHL:
    case TOK_SHR:
    case TOK_MOD:
    case TOK_MOD_ASSIGN:
    case TOK_BIT_OR_ASSIGN:
    case TOK_BIT_AND_ASSIGN:
    case TOK_XOR_ASSIGN:
    case TOK_SHL_ASSIGN:
    case TOK_SHR_ASSIGN: /// Fall through.
        are_correct |= t == D_T_INT;
        are_correct |= t == D_T_CHAR;
        are_correct |= t == D_T_BOOL;
        break;
    default:
        break;
    }

    return are_correct;
}

static void visit_ast_binary(struct ast_node *ast)
{
    struct ast_binary *stmt = ast->ast;

    visit_ast_node(stmt->lhs);
    enum data_type l_dt = last_dt;
    uint16_t l_indir_lvl = last_indir_lvl;

    visit_ast_node(stmt->rhs);
    enum data_type r_dt = last_dt;
    uint16_t r_indir_lvl = last_indir_lvl;

    bool are_same = false;
    are_same |= l_dt == D_T_BOOL && r_dt == D_T_BOOL;
    are_same |= l_dt == D_T_CHAR && r_dt == D_T_CHAR;
    are_same |= l_dt == D_T_FLOAT && r_dt == D_T_FLOAT;
    are_same |= l_dt == D_T_INT && r_dt == D_T_INT;

    if (l_indir_lvl == 0 && r_indir_lvl == 0) {
        bool correct_ops = correct_bin_ops(stmt->operation, l_dt);
        if (!are_same || !correct_ops)
            weak_compile_error(
                ast->line_no,
                ast->col_no,
                "Cannot apply `%s` to %s and %s",
                tok_to_string(stmt->operation),
                data_type_to_string(l_dt),
                data_type_to_string(r_dt)
            );
    } else {
        bool correct_ops = l_indir_lvl == r_indir_lvl;
        if (!are_same || !correct_ops)
            weak_compile_error(
                ast->line_no,
                ast->col_no,
                "Indirection level mismatch (%d vs %d)",
                l_indir_lvl,
                r_indir_lvl
            );
    }
}

static void visit_ast_unary(struct ast_node *ast)
{
    struct ast_unary *stmt = ast->ast;
    visit_ast_node(stmt->operand);
    enum data_type dt = last_dt;

    switch (stmt->operation) {
    case TOK_INC:
    case TOK_DEC: /// Fall through.
        if (dt != D_T_CHAR && dt != D_T_INT)
            weak_compile_error(
                ast->line_no,
                ast->col_no,
                "Cannot apply `%s` to %s",
                tok_to_string(stmt->operation),
                data_type_to_string(dt)
            );
        break;
    case TOK_BIT_AND: /// Address operator `&`.
        ++last_indir_lvl;
        break;
    case TOK_STAR: /// Dereference operator `*`.
        if (last_indir_lvl == 0)
            weak_compile_error(
                ast->line_no,
                ast->col_no,
                "Attempt to dereference integral type"
            );
        --last_indir_lvl;
        break;
    default:
        weak_unreachable("Invalid unary operand.");
    }
}

static void visit_ast_symbol(struct ast_node *ast)
{
    struct ast_symbol *stmt = ast->ast;
    struct ast_storage_decl *record = ast_storage_lookup(stmt->value);

    last_dt = record->data_type;
    last_indir_lvl = record->indirection_lvl;
}

static void visit_ast_var_decl(struct ast_node *ast)
{
    struct ast_var_decl *decl = ast->ast;
    if (decl->body) {
        visit_ast_node(decl->body);
        if (decl->dt != last_dt)
            weak_compile_error(
                ast->line_no,
                ast->col_no,
                "Cannot assign %s to variable of type %s",
                data_type_to_string(last_dt),
                data_type_to_string(decl->dt)
            );
    }
    ast_storage_push_typed(decl->name, decl->dt, decl->indirection_lvl, ast);
    last_dt = decl->dt;
    last_indir_lvl = decl->indirection_lvl;
}

static void visit_ast_array_decl(struct ast_node *ast)
{
    struct ast_array_decl *decl = ast->ast;
    /// Required to be compound.
    struct ast_compound *dimensions = decl->enclosure_list->ast;
    for (uint64_t i = 0; i < dimensions->size; ++i) {
        int32_t num = ( (struct ast_num *) (dimensions->stmts[i]->ast) )->value;
        if (num == 0)
            weak_compile_error(
                ast->line_no,
                ast->col_no,
                "Array size cannot be equal '0'"
            );
    }

    ast_storage_push_typed(decl->name, decl->dt, decl->indirection_lvl, ast);
    last_dt = decl->dt;
    last_indir_lvl = decl->indirection_lvl;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
static void out_of_range_analysis(struct ast_node *decl_indices_ast, struct ast_node *indices_ast)
{
    struct ast_compound *call_indices = indices_ast->ast;
    struct ast_compound *decl_indices = decl_indices_ast->ast;
    assert(call_indices->size);
    assert(decl_indices->size);
    if (decl_indices->size < call_indices->size) {
        char index[16];
        ordinal_numeral(call_indices->size, index);
        weak_compile_error(
            call_indices->stmts[0]->line_no,
            call_indices->stmts[0]->col_no,
            "Cannot get %s index of %d dimensional array",
            index, decl_indices->size
        );
    }

    for (uint64_t i = 0; i < MIN(call_indices->size, decl_indices->size); ++i) {
        if (call_indices->stmts[i]->type != AST_INTEGER_LITERAL)
            continue;
        struct ast_node *index_ast = call_indices->stmts[i];
        struct ast_num *num = index_ast->ast;
        struct ast_num *decl_index_ast = decl_indices->stmts[i]->ast;
        int32_t index = num->value;
        int32_t decl_index = decl_index_ast->value;

        if (index < 0)
            weak_compile_error(
                index_ast->line_no,
                index_ast->col_no,
                "Array index less than zero"
            );

        if (index >= decl_index)
            weak_compile_error(
                index_ast->line_no,
                index_ast->col_no,
                "Out of range! Index (which is %d) >= array size (which is %d)",
                index, decl_index
            );
    }
}
#undef MIN

static void visit_ast_array_access(struct ast_node *ast)
{
    struct ast_array_access *stmt = ast->ast;
    struct ast_node *record = ast_storage_lookup(stmt->name)->ast;

    if (record->type != AST_ARRAY_DECL) {
        /// If it is not an array, then obviously variable
        /// declaration.
        struct ast_var_decl *decl = record->ast;
        if (decl->indirection_lvl == 0)
            weak_compile_error(
                ast->line_no,
                ast->col_no,
                "Cannot get index of non-array type"
            );
        last_dt = decl->dt;
    } else {
        struct ast_array_decl *decl = record->ast;
        out_of_range_analysis(decl->enclosure_list, stmt->indices);
    }
    struct ast_compound *enclosure = stmt->indices->ast;
    for (uint64_t i = 0; i < enclosure->size; ++i) {
        visit_ast_node(enclosure->stmts[i]);
        if (last_dt != D_T_INT)
            weak_compile_error(
                enclosure->stmts[i]->line_no,
                enclosure->stmts[i]->col_no,
                "Expected integer as array index, got %s",
                data_type_to_string(last_dt)
            );
    }
}

static void require_last_dt_convertible_to_bool(struct ast_node *location)
{
    enum data_type dt = last_dt;
    if (dt != D_T_INT && dt != D_T_BOOL)
        weak_compile_error(
            location->line_no,
            location->col_no,
            "Cannot convert %s to boolean",
            data_type_to_string(dt)
        );
}

static void visit_ast_if(struct ast_node *ast)
{
    struct ast_if *stmt = ast->ast;
    visit_ast_node(stmt->condition);
    require_last_dt_convertible_to_bool(ast);

    visit_ast_node(stmt->body);
    if (stmt->else_body)
        visit_ast_node(stmt->else_body);
}

static void visit_ast_for(struct ast_node *ast)
{
    struct ast_for *stmt = ast->ast;
    if (stmt->init)
        visit_ast_node(stmt->init);
    if (stmt->condition) {
        visit_ast_node(stmt->condition);
        require_last_dt_convertible_to_bool(ast);
    }
    if (stmt->increment)
        visit_ast_node(stmt->increment);
    visit_ast_node(stmt->body);
}

static void visit_ast_while(struct ast_node *ast)
{
    struct ast_while *stmt = ast->ast;
    visit_ast_node(stmt->condition);
    require_last_dt_convertible_to_bool(ast);
    visit_ast_node(stmt->body);
}

static void visit_ast_do_while(struct ast_node *ast)
{
    struct ast_do_while *stmt = ast->ast;
    visit_ast_node(stmt->body);
    visit_ast_node(stmt->condition);
    require_last_dt_convertible_to_bool(ast);
}

static void visit_ast_return(struct ast_node *ast)
{
    struct ast_return *stmt = ast->ast;
    if (stmt->operand)
        visit_ast_node(stmt->operand);
    last_return_dt = last_dt;
}

static void visit_ast_compound(struct ast_node *ast)
{
    struct ast_compound *stmt = ast->ast;
    ast_storage_start_scope();
    for (uint64_t i = 0; i < stmt->size; ++i)
        visit_ast_node(stmt->stmts[i]);
    ast_storage_end_scope();
}

static char *decl_name(struct ast_node *decl)
{
    if (decl->type == AST_VAR_DECL)
        return ( (struct ast_var_decl *) decl->ast )->name;

    if (decl->type == AST_ARRAY_DECL)
        return ( (struct ast_array_decl *) decl->ast )->name;

    weak_unreachable("Declaration expected.");
}

static void visit_ast_function_call(struct ast_node *ast)
{
    struct ast_function_call *call = ast->ast;
    struct ast_node *decl = ast_storage_lookup(call->name)->ast;
    if (decl->type != AST_FUNCTION_DECL)
        weak_compile_error(
            ast->line_no,
            ast->col_no,
            "`%s` is not a function",
            call->name
        );

    struct ast_function_decl *fun = decl->ast;
    struct ast_compound *fun_args = fun->args->ast;
    struct ast_compound *call_args = call->args->ast;
    assert(
        fun_args->size == call_args->size &&
        "Call arguments size checked in function analyzer."
    );

    for (uint64_t i = 0; i < call_args->size; ++i) {
        visit_ast_node(fun_args->stmts[i]);
        enum data_type l_dt = last_dt;
        uint64_t l_indir_lvl = last_indir_lvl;

        visit_ast_node(call_args->stmts[i]);
        enum data_type r_dt = last_dt;
        uint64_t r_indir_lvl = last_indir_lvl;

        if (l_dt != r_dt)
            weak_compile_error(
                call_args->stmts[i]->line_no,
                call_args->stmts[i]->col_no,
                "For argument `%s` got %s, but %s expected",
                decl_name(fun_args->stmts[i]),
                data_type_to_string(r_dt),
                data_type_to_string(l_dt)
            );

        if (l_indir_lvl != r_indir_lvl)
            weak_compile_error(
                ast->line_no,
                ast->col_no,
                "Indirection level mismatch (%d vs %d)",
                l_indir_lvl,
                r_indir_lvl
            );
    }
    last_dt = fun->data_type;
    last_indir_lvl = fun->indirection_lvl;
}

static void visit_ast_function_decl(struct ast_node *ast)
{
    struct ast_function_decl *decl = ast->ast;
    enum data_type dt = decl->data_type;
    if (decl->body == NULL) { /// Function prototype.
        ast_storage_push_typed(decl->name, D_T_FUNC, decl->indirection_lvl, ast);
        return;
    }
    ast_storage_start_scope();
    /// This is to have function in recursive calls.
    ast_storage_push_typed(decl->name, D_T_FUNC, decl->indirection_lvl, ast);
    /// Don't just visit compound AST, which creates and terminates scope.
    struct ast_compound *args = decl->args->ast;
    for (uint64_t i = 0; i < args->size; ++i)
        visit_ast_node(args->stmts[i]);

    visit_ast_node(decl->body);
    if (dt != D_T_VOID && dt != last_return_dt)
        weak_compile_error(
            ast->line_no,
            ast->col_no,
            "Cannot return %s instead of %s",
            data_type_to_string(last_return_dt),
            data_type_to_string(dt)
        );
    ast_storage_end_scope();
    /// This is to have function outside.
    ast_storage_push_typed(decl->name, D_T_FUNC, decl->indirection_lvl, ast);
}

void visit_ast_node(struct ast_node *ast)
{
    assert(ast);

    switch (ast->type) {
    case AST_MEMBER: /// Unused... Or should be used?
    case AST_STRUCT_DECL: /// Unused... Or should be used?
    case AST_BREAK_STMT: /// Unused.
    case AST_CONTINUE_STMT: break; /// Unused.
    case AST_CHAR_LITERAL:
        visit_ast_char();
        break;
    case AST_INTEGER_LITERAL:
        visit_ast_num();
        break;
    case AST_FLOATING_POINT_LITERAL:
        visit_ast_float();
        break;
    case AST_STRING_LITERAL:
        visit_ast_string();
        break;
    case AST_BOOLEAN_LITERAL:
        visit_ast_bool();
        break;
    case AST_SYMBOL:
        visit_ast_symbol(ast);
        break;
    case AST_VAR_DECL:
        visit_ast_var_decl(ast);
        break;
    case AST_ARRAY_DECL:
        visit_ast_array_decl(ast);
        break;
    case AST_BINARY:
        visit_ast_binary(ast);
        break;
    case AST_PREFIX_UNARY:
        visit_ast_unary(ast);
        break;
    case AST_POSTFIX_UNARY:
        visit_ast_unary(ast);
        break;
    case AST_ARRAY_ACCESS:
        visit_ast_array_access(ast);
        break;
    case AST_IF_STMT:
        visit_ast_if(ast);
        break;
    case AST_FOR_STMT:
        visit_ast_for(ast);
        break;
    case AST_WHILE_STMT:
        visit_ast_while(ast);
        break;
    case AST_DO_WHILE_STMT:
        visit_ast_do_while(ast);
        break;
    case AST_RETURN_STMT:
        visit_ast_return(ast);
        break;
    case AST_COMPOUND_STMT:
        visit_ast_compound(ast);
        break;
    case AST_FUNCTION_DECL:
        visit_ast_function_decl(ast);
        break;
    case AST_FUNCTION_CALL:
        visit_ast_function_call(ast);
        break;
    default:
        weak_unreachable("Unknown AST type (numeric: %d).", ast->type);
    }
}

void analysis_type_analysis(struct ast_node *root)
{
    ast_storage_init_state();
    visit_ast_node(root);
    reset_internal_state();
    ast_storage_reset_state();
}
