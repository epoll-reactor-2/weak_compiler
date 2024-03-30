/* pp.c - Preprocessor.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/pp.h"
#include "util/diagnostic.h"
#include <string.h>

static tokens_t tokens;
struct token current_token;

extern FILE *yyin;
extern int yylex();
extern int yylex_destroy();

void lex_token(struct token *t)
{
    memcpy(&current_token, t, sizeof (*t));
}

static struct token *peek_current()
{
    return &current_token;
}

/* TODO: Such API
         1) Clear stack
         2) peek_next(2)
            \
             Lex 2 elements, next process.
         
         Don't keep everything for no reason. */
static struct token *peek_next()
{
    if (yylex() <= 0)
        return NULL;
    return peek_current();
}

static struct token *require_token(enum token_type t)
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

static struct token *require_char(char c)
{
    return require_token(tok_char_to_tok(c));
}

noreturn static void report_unexpected(struct token *t)
{
    fcc_compile_error(t->line_no, t->col_no, "Unexpected token `%s`", tok_to_string(t->type));
}

/**********************************************
 **            Preprocessor                  **
 **********************************************/

static vector_t(char *) pp_paths;

void pp_init()
{
    static char *p[] = {
        "/usr/include",
        "/usr/include/bits",
        "/usr/include/linux",
        "/usr/include/c++/13.2.1",
        "/usr/include/c++/13.2.1/tr1",
        "/usr/include/c++/13.2.1/bits",
        "/usr/include/c++/13.2.1/x86_64-pc-linux-gnu",
        "/usr/include/x86_64-linux-gnu",
        NULL
    };
    char **it = p;

    while (*it)
        vector_push_back(pp_paths, strdup(*it++));
}

void pp_deinit()
{
    vector_foreach(pp_paths, i) {
        char *s = vector_at(pp_paths, i);
        free(s);
    }
    vector_free(pp_paths);
}

void pp_add_include_path(const char *path)
{
    printf("PP: adding %s\n", path);
    vector_push_back(pp_paths, strdup(path));
}

static FILE *pp_try_open(const char *filename)
{
    char path[512] = {0};

    vector_foreach(pp_paths, i) {
        const char *pp_path = vector_at(pp_paths, i);
        snprintf(path, sizeof (path) - 1, "%s/%s", pp_path, filename);

        printf("PP: Searching %s\n", path);

        FILE *f = fopen(path, "rb");
        if (f)
            return f;
    }

    printf("Cannot open file %s\n", filename);
    exit(-1);
}

/**********************************************
 **                #include                  **
 **********************************************/

static void pp_include_path_user(char *path)
{
    strcpy(path, peek_current()->data);
}

static void pp_include_path_system(char *path)
{
    struct token *t = peek_next();
    while (!tok_is(t, '>')) {
        path = strcat(path, t->data ? t->data : tok_to_string(t->type));
        t = peek_next();
    }

    require_char('>');
}

static void pp_include()
{
    struct token *t = peek_next();

    bool is_system = tok_is(t, '<');
    bool is_user   = t->type == T_STRING_LITERAL;

    if (!is_system && !is_user)
        report_unexpected(t);

    char path[512] = {0};

    if (is_user)   pp_include_path_user(path);
    if (is_system) pp_include_path_system(path);

    pp(path);
}

/**********************************************
 **                #define                   **
 **********************************************/

unused static void pp_define_macro(unused struct token *t)
{}

unused static void pp_define_id(unused struct token *t)
{}

/* 1. #define macro
   2. #define macro(...) */
static void pp_define()
{
    struct token *t = peek_next();

    if (t->type == T_MACRO)
        pp_define_macro(t);
    else
        pp_define_id(t);

    while (1) {
        switch (peek_current()->type) {
        case T_BACKSLASH:
            peek_next();
            break;
        case T_NEWLINE:
            goto out;
        default:
            peek_next();
            break;
        }
    }

out:
    return;
}

static void pp_directive()
{
    struct token *t = peek_next();

    switch (t->type) {
    /* 6.10 if-group */
    case T_IFDEF:
        break;
    case T_IFNDEF:
        break;
    case T_IF:
        break;
    /* 6.10 elif-groups */
    case T_ELIF:
        break;
    /* 6.10 endif-line */
    case T_ENDIF:
        break;
    /* 6.10 control-line */
    case T_INCLUDE:
        pp_include();
        break;
    case T_DEFINE:
        pp_define();
        break;
    case T_UNDEF:
        break;
    case T_LINE:
        break;
    case T_ERROR:
        break;
    case T_PRAGMA:
        break;
    default:
        report_unexpected(t);
    }
}

static void pp_read()
{
    struct token *t = peek_next();

    while (t) {
        switch (t->type) {
        case T_HASH:
            pp_directive();
            break;
        default:
            /* Rest of tokens dedicated for parser not preprocessor. */
            vector_push_back(tokens, *t);
            break;
        }
        t = peek_next();
    }
}

tokens_t *pp(const char *filename)
{
    char  *_  = NULL;
    size_t __ = 0;
    yyin = open_memstream(&_, &__);

    FILE *f = pp_try_open(filename);

    char line[512] = {0};
    while (1) {
        if (!fgets(line, 512, f)) {
            fclose(f);
            break;
        }

        fwrite(line, strlen(line), 1, yyin);

        /* TODO: Now theoretically parser can operate on one
                 source code fragment of some small size. As an
                 option, we can put the whole source tokens into
                 one token table and start to parse with it. */
        pp_read();
    }

    return &tokens;
}