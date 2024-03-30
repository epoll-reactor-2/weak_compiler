/* parse.c - Syntax analyzer.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/ast.h"
#include "front_end/data_type.h"
#include "front_end/parse.h"
#include "front_end/tok.h"
#include "front_end/pp.h"
#include "util/alloc.h"
#include "util/diagnostic.h"
#include "util/unreachable.h"
#include "util/vector.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

/**********************************************
 **              Lex callback                **
 **********************************************/

/* We use stack because C grammar may require from us
   lookahead by few tokens. */
static tokens_t *tokens;
static uint64_t  token_idx;

static struct token *peek_current()
{
    return &vector_at(*tokens, token_idx);
}

/* TODO: Such API
         1) Clear stack
         2) peek_next(2)
            \
             Lex 2 elements, next process.
         
         Don't keep everything for no reason. */
static struct token *peek_next()
{
    struct token *t = peek_current();
    ++token_idx;
    return t;
}

/**********************************************
 **                Parser                    **
 **********************************************/

typedef vector_t(struct ast_node *) ast_array_t;

struct localized_data_type {
    enum data_type  data_type;
    char           *type_name;
    uint16_t        ptr_depth;
    uint16_t        line_no;
    int16_t         col_no;
};

noreturn static void report_unexpected(struct token *t)
{
    fcc_compile_error(t->line_no, t->col_no, "Unexpected token `%s`", tok_to_string(t->type));
}

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
    
    peek_next();

    return curr_tok;
}

struct token *require_char(char c)
{
    return require_token(tok_char_to_tok(c));
}

unused static enum data_type tok_to_data_type(enum token_type t)
{
    switch (t) {
    case T_VOID:   return D_T_VOID;
    case T_INT:    return D_T_INT;
    case T_FLOAT:  return D_T_FLOAT;
    case T_CHAR:   return D_T_CHAR;
    case T_BOOL:   return D_T_BOOL;
    case T_SYM:    return D_T_STRUCT;
    default:
        fcc_unreachable(
            "Cannot convert token `%s` to the data type",
            tok_to_string(t)
        );
    }
}

/* https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1548.pdf */
unused static enum token_type             parse_punctuator(); /* 6.4.6 */
unused static struct ast_node            *parse_storage_class_specifier(); /* 6.7.1 */
unused static struct localized_data_type  parse_type_specifier(); /* 6.7.2 */
unused static enum token_type             parse_struct_or_union(); /* 6.7.2.1 */
unused static struct ast_node            *parse_enum_specifier(); /* 6.7.2.2 */
unused static enum token_type             parse_type_qualifier(); /* 6.7.3 */
unused static enum token_type             parse_function_specifier(); /* 6.7.4 */
unused static struct ast_node            *parse_translation_unit(); /* 6.9 */

static enum token_type parse_punctuator() /* 6.4.6 */
{
    struct token *c = peek_next();
    switch (c->type) {
    case T_OPEN_BRACKET: /* [ */
    case T_CLOSE_BRACKET: /* ] */
    case T_OPEN_PAREN: /* ( */
    case T_CLOSE_PAREN: /* ) */
    case T_OPEN_BRACE: /* { */
    case T_CLOSE_BRACE: /* } */
    case T_DOT: /* . */
    case T_ARROW: /* -> */

    case T_INC: /* ++ */
    case T_DEC: /* -- */
    case T_BIT_AND: /* & */
    case T_STAR: /* * */
    case T_PLUS: /* + */
    case T_MINUS: /* - */
    case T_TILDE: /* ~ */
    case T_EXCLAMATION: /* ! */

    case T_SLASH: /* / */
    case T_MOD: /* % */
    case T_SHL: /* << */
    case T_SHR: /* >> */
    case T_LT: /* < */
    case T_GT: /* > */
    case T_LE: /* <= */
    case T_GE: /* >= */
    case T_EQ: /* == */
    case T_NEQ: /* != */
    case T_BIT_XOR: /* ^ */
    case T_BIT_OR: /* | */
    case T_AND: /* && */
    case T_OR: /* || */

    case T_QUESTION_MARK: /* ? */
    case T_COLON: /* : */
    case T_SEMICOLON: /* ; */
    case T_ELLIPSIS: /* ... */

    case T_ASSIGN: /* = */
    case T_MUL_ASSIGN: /* *= */
    case T_DIV_ASSIGN: /* /= */
    case T_MOD_ASSIGN: /* %= */
    case T_PLUS_ASSIGN: /* += */
    case T_MINUS_ASSIGN: /* -= */
    case T_SHL_ASSIGN: /* <<= */
    case T_SHR_ASSIGN: /* >>= */
    case T_BIT_AND_ASSIGN: /* &= */
    case T_BIT_XOR_ASSIGN: /* &= */
    case T_BIT_OR_ASSIGN: /* |= */

    case T_COMMA: /* , */
    case T_HASH: /* # */
    case T_HASH_HASH: /* ## */
        return c->type;
    default:
        report_unexpected(c);
    }
}

static struct ast_node *parse_storage_class_specifier() /* 6.7.1 */
{
    struct token *c = peek_next();
    switch (c->type) {
    case T_TYPEDEF:
    case T_EXTERN:
    case T_STATIC:
    case T_THREAD_LOCAL:
    case T_AUTO:
    case T_REGISTER:
        return NULL;
    default:
        report_unexpected(c);
    }
}

static struct localized_data_type parse_type_specifier() /* 6.7.2 */
{
    struct token *c = peek_next();
    struct localized_data_type dt = {
        .col_no  = c->col_no,
        .line_no = c->line_no,
    };
    enum data_type t = 0;

    switch (c->type) {
    case T_VOID: t = D_T_VOID; break;
    case T_CHAR: t = D_T_CHAR; break;
    case T_SHORT: t = D_T_SHORT; break;
    case T_INT: t = D_T_INT; break;
    case T_LONG: t = D_T_LONG; break;
    case T_FLOAT: t = D_T_FLOAT; break;
    case T_DOUBLE: t = D_T_DOUBLE; break;
    case T_SIGNED: t = D_T_SIGNED; break;
    case T_UNSIGNED: t = D_T_UNSIGNED; break;
    case T_BOOL: t = D_T_BOOL; break;
    case T_COMPLEX: t = D_T_COMPLEX; break;
    default:
        report_unexpected(c);
    }
    dt.data_type = t;

    return dt;
}

static enum token_type parse_struct_or_union() /* 6.7.2.1 */
{
    struct token *c = peek_next();
    switch (c->type) {
    case T_STRUCT:
    case T_UNION:
        return c->type;
    default:
        report_unexpected(c);
    }
}

static struct ast_node *parse_enum_specifier() /* 6.7.2.2 */
{
    require_token(T_ENUM);
    struct token *c = peek_current();

    switch (c->type) {
    case T_SYM:
        return NULL;
    case T_OPEN_BRACE:
        return NULL;
    default:
        report_unexpected(c);
    }
}

static enum token_type parse_type_qualifier() /* 6.7.3 */
{
    struct token *c = peek_next();
    switch (c->type) {
    case T_CONST:
    case T_RESTRICT:
    case T_VOLATILE:
    case T_ATOMIC:
        return c->type;
    default:
        report_unexpected(c);
    }
}

static enum token_type parse_function_specifier() /* 6.7.4 */
{
    struct token *c = peek_next();
    switch (c->type) {
    case T_INLINE:
    case T_NORETURN:
        return c->type;
    default:
        report_unexpected(c);
    }
}

static struct ast_node *parse_translation_unit() /* 6.9 */
{
    return NULL;
}

/**********************************************
 **                Parser                    **
 **********************************************/

struct ast_node *parse(const char *filename)
{
    tokens = pp(filename);

    vector_foreach(*tokens, i) {
        struct token *t = &vector_at(*tokens, i);
        printf("Consume %s %s\n", tok_to_string(t->type), t->data);
    }

    return NULL;
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