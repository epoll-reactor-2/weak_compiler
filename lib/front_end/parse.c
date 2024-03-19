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

static void report_unexpected(struct token *t)
{
    fcc_compile_error(t->line_no, t->col_no, "Unexpected token `%s`", tok_to_string(t->type));
}

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

/* https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1548.pdf */
static enum token_type             parse_punctuator(); /* 6.4.6 */
static struct ast_node            *parse_q_chars(); /* 6.4.7 */
static struct ast_node            *parse_h_chars(); /* 6.4.7 */
static struct ast_node            *parse_storage_class_specifier(); /* 6.7.1 */
static struct localized_data_type  parse_type_specifier(); /* 6.7.2 */
static enum token_type             parse_struct_or_union(); /* 6.7.2.1 */
static struct ast_node            *parse_enum_specifier(); /* 6.7.2.2 */
static enum token_type             parse_type_qualifier(); /* 6.7.3 */
static enum token_type             parse_function_specifier(); /* 6.7.4 */

static enum token_type parse_punctuator() /* 6.4.6 */
{
    struct token *c = peek_next();
    switch (c->type) {
    case TOK_OPEN_BRACKET: /* [ */
    case TOK_CLOSE_BRACKET: /* ] */
    case TOK_OPEN_PAREN: /* ( */
    case TOK_CLOSE_PAREN: /* ) */
    case TOK_OPEN_BRACE: /* { */
    case TOK_CLOSE_BRACE: /* } */
    case TOK_DOT: /* . */
    case TOK_ARROW: /* -> */

    case TOK_INC: /* ++ */
    case TOK_DEC: /* -- */
    case TOK_BIT_AND: /* & */
    case TOK_STAR: /* * */
    case TOK_PLUS: /* + */
    case TOK_MINUS: /* - */
    case TOK_TILDE: /* ~ */
    case TOK_EXCLAMATION: /* ! */

    case TOK_SLASH: /* / */
    case TOK_MOD: /* % */
    case TOK_SHL: /* << */
    case TOK_SHR: /* >> */
    case TOK_LT: /* < */
    case TOK_GT: /* > */
    case TOK_LE: /* <= */
    case TOK_GE: /* >= */
    case TOK_EQ: /* == */
    case TOK_NEQ: /* != */
    case TOK_BIT_XOR: /* ^ */
    case TOK_BIT_OR: /* | */
    case TOK_AND: /* && */
    case TOK_OR: /* || */

    case TOK_QUESTION_MARK: /* ? */
    case TOK_COLON: /* : */
    case TOK_SEMICOLON: /* ; */
    case TOK_ELLIPSIS: /* ... */

    case TOK_ASSIGN: /* = */
    case TOK_MUL_ASSIGN: /* *= */
    case TOK_DIV_ASSIGN: /* /= */
    case TOK_MOD_ASSIGN: /* %= */
    case TOK_PLUS_ASSIGN: /* += */
    case TOK_MINUS_ASSIGN: /* -= */
    case TOK_SHL_ASSIGN: /* <<= */
    case TOK_SHR_ASSIGN: /* >>= */
    case TOK_BIT_AND_ASSIGN: /* &= */
    case TOK_BIT_XOR_ASSIGN: /* &= */
    case TOK_BIT_OR_ASSIGN: /* |= */

    case TOK_COMMA: /* , */
    case TOK_HASH: /* # */
    case TOK_HASH_HASH: /* ## */
        return c->type;
    default:
        report_unexpected(c);
    }
}

static struct ast_node *parse_storage_class_specifier() /* 6.7.1 */
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
        report_unexpected(c);
    }

    return dt;
}

static enum token_type parse_struct_or_union() /* 6.7.2.1 */
{
    struct token *c = peek_next();
    switch (c->type) {
    case TOK_STRUCT:
    case TOK_UNION:
        return c->type;
    default:
        report_unexpected(c);
    }
}

static struct ast_node *parse_enum_specifier() /* 6.7.2.2 */
{
    require_token(TOK_ENUM);
    struct token *c = peek_current();

    switch (c->type) {
    case TOK_SYM:
        return NULL;
    case TOK_OPEN_BRACE:
        return NULL;
    default:
        report_unexpected(c);
    }
}

static enum token_type parse_type_qualifier() /* 6.7.3 */
{
    struct token *c = peek_next();
    switch (c->type) {
    case TOK_CONST:
    case TOK_RESTRICT:
    case TOK_VOLATILE:
    case TOK_ATOMIC:
        return c->type;
    default:
        report_unexpected(c);
    }
}

static enum token_type parse_function_specifier() /* 6.7.4 */
{
    struct token *c = peek_next();
    switch (c->type) {
    case TOK_INLINE:
    case TOK_NORETURN:
        return c->type;
    default:
        report_unexpected(c);
    }
}

struct ast_node *parse_tokens(const struct token *begin, const struct token *end)
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

/**********************************************
 **             Preprocessor                 **
 **********************************************/

/* TODO: -I option should append there. */
static char *pp_default_paths[] = {
    /* TODO: This depends on testing semantics.
             Better to write shell scripts. */
    "/home/fuck/fcc/build/inputs/parser",
    /* TODO: Recursive search. */
    "/usr/include",
    "/usr/include/bits",
    "/usr/include/linux",
    "/usr/include/c++/13.2.1",
    "/usr/include/c++/13.2.1/tr1",
    "/usr/include/c++/13.2.1/bits",
    "/usr/include/c++/13.2.1/x86_64-pc-linux-gnu"
};

FILE *pp_try_open(const char *filename)
{
    char path[512] = {0};

    for (uint64_t i = 0; i < __fcc_array_size(pp_default_paths); ++i) {
        snprintf(path, sizeof (path) - 1, "%s/%s", pp_default_paths[i], filename);

        printf("PP: Searching %s\n", path);

        FILE *f = fopen(path, "rb");
        if (f)
            return f;
    }

    printf("Cannot open file %s\n", filename);
    exit(-1);
}

static void preprocess(const char *filename)
{
    char line[8192];

    FILE *f = pp_try_open(filename);

    while (1) {
        if (!fgets(line, sizeof (line), f)) {
            fclose(f);
            return;
        }

        char *line_ptr = line;
        while (isspace(*line_ptr))
            ++line_ptr;

        printf("%s", line_ptr);

        int pp_include_len = sizeof ("#include") - 1;

        if (strncmp(line_ptr, "#include", pp_include_len))
            continue;

        line_ptr += pp_include_len;

        /* line_ptr[0] must be " or <. */

        while (isspace(*line_ptr))
            ++line_ptr;

        /* Consume " or <. */
        ++line_ptr;

        /* Cut " or > from include path. */
        char *old_line_ptr = line_ptr;
        while (*line_ptr != '"' && *line_ptr != '>')
            ++line_ptr;
        *line_ptr = '\0';

        line_ptr = old_line_ptr;

        preprocess(line_ptr);
    }
}

/**********************************************
 **                Parser                    **
 **********************************************/

struct ast_node *parse(const char *filename)
{
    puts("");
    preprocess(filename);
    fflush(stdout);
    exit(1);
    return parse_tokens(NULL, NULL);
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