/* parse.c - Syntax analyzer.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/ast.h"
#include "front_end/data_type.h"
#include "front_end/parse.h"
#include "util/alloc.h"
#include "util/diagnostic.h"
#include "util/unreachable.h"
#include "util/vector.h"
#include <assert.h>
#include <string.h>

typedef vector_t(struct ast_node *) ast_array_t;

struct localized_data_type {
    enum data_type  data_type;
    char           *type_name;
    uint16_t        ptr_depth;
    uint16_t        line_no;
    int16_t         col_no;
};

static struct token *tok_begin;
static struct token *tok_end;
static uint32_t      loops_depth = 0;

static struct token *peek_current() { return tok_begin;   }
static struct token *peek_next   () { return tok_begin++; }

struct token *require_token(enum token_type t)
{
    struct token *curr_tok = peek_current();

    if (curr_tok->type != t)
        fcc_compile_error(
            curr_tok->line_no,
            curr_tok->col_no,
            "Expected `%s`, got `%s`",
            tok_to_string(t), tok_to_string(curr_tok->type)
        );

    ++tok_begin;
    return curr_tok;
}

struct token *require_char(char c)
{
    return require_token(tok_char_to_tok(c));
}

static enum data_type tok_to_data_type(enum token_type t)
{
    switch (t) {
    case TOK_VOID:   return D_T_VOID;
    case TOK_INT:    return D_T_INT;
    case TOK_FLOAT:  return D_T_FLOAT;
    case TOK_CHAR:   return D_T_CHAR;
    case TOK_BOOL:   return D_T_BOOL;
    case TOK_SYM:    return D_T_STRUCT;
    default:
        fcc_unreachable(
            "Cannot convert token `%s` to the data type",
            tok_to_string(t)
        );
    }
}

static struct ast_node            *parse_q_chars(); /* "Quoted header chars". */
static struct ast_node            *parse_h_chars(); /* <Braced header chars>. */
static struct ast_node            *parse_storage_class_specifier();
static struct localized_data_type  parse_type_specifier();

struct ast_node *parse(const struct token *begin, const struct token *end)
{
    tok_begin = (struct token *) begin;
    tok_end   = (struct token *) end;

    typedef vector_t(struct ast_node *) stmts_t;

    stmts_t global_stmts = {0};
    struct token *curr = NULL;

    while (tok_begin < tok_end) {
        curr = peek_next();
        switch (curr->type) {
        case TOK_TYPEDEF:
        case TOK_STRUCT:
            // vector_push_back(global_stmts, parse_struct_decl());
            break;
        case TOK_VOID:
        case TOK_INT:
        case TOK_CHAR:
        case TOK_FLOAT:
        case TOK_BOOL: /* Fall through. */
            // vector_push_back(global_stmts, parse_function_decl());
            break;
        default:
            fcc_compile_error(
                curr->line_no,
                curr->col_no,
                "Unexpected token in global context: %s\n",
                tok_to_string(curr->type)
            );
        }
    }

    return ast_compound_init(
        /*size=*/global_stmts.count,
        /*stmts=*/global_stmts.data,
        /*line_no=*/0,
        /*col_no=*/0
    );
}

static struct ast_node *parse_storage_class_specifier()
{
    struct token *c = peek_next();
    switch (c->type) {
    case TOK_TYPEDEF:
    case TOK_EXTERN:
    case TOK_STATIC:
    case TOK_THREAD_LOCAL:
    case TOK_AUTO:
    case TOK_REGISTER:
        return NULL;
    default:
        fcc_compile_error(c->line_no, c->col_no, "");
    }
}

static struct localized_data_type parse_type_specifier()
{
    struct token *c = peek_next();
    struct localized_data_type dt = {
        .col_no  = c->col_no,
        .line_no = c->line_no,
    };
    enum data_type t = 0;

    switch (c->type) {
    case TOK_VOID: t = D_T_VOID; break;
    case TOK_CHAR: t = D_T_CHAR; break;
    case TOK_SHORT: t = D_T_SHORT; break;
    case TOK_INT: t = D_T_INT; break;
    case TOK_LONG: t = D_T_LONG; break;
    case TOK_FLOAT: t = D_T_FLOAT; break;
    case TOK_DOUBLE: t = D_T_DOUBLE; break;
    case TOK_SIGNED: t = D_T_SIGNED; break;
    case TOK_UNSIGNED: t = D_T_UNSIGNED; break;
    case TOK_BOOL: t = D_T_BOOL; break;
    case TOK_COMPLEX: t = D_T_COMPLEX; break;
    default:
        fcc_compile_error(c->line_no, c->col_no, "");
    }

    return dt;
}

/*
Однажды болезнями, стонами, страхами затаёнными
Ты придёшь на голос мой - я позову.
Тропы родными протоптаны,
Мрамор высечен, ямы закопаны.
С головой под землёй в нижнем ряду.
Горя слезами невечными,
Зеркалами завешанными
Ты пойдёшь вслед за мной - я провожу.
*/