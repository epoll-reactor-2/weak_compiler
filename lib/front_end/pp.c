/* pp.c - Preprocessor.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/pp.h"
#include "util/diagnostic.h"
#include "util/unreachable.h"
#include <string.h>

static tokens_t tokens;
struct token current_token;
static vector_t(char *) pp_paths;

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

void pp_init()
{
    /* There you can add your fancy #include paths. */
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

static void cpp(const char *full_path, long siz)
{
    char cmd [512] = {0};
    char line[512] = {0};

    char   *__memstream = NULL;
    size_t  __memstream_size = 0;
    yyin = open_memstream(&__memstream, &__memstream_size);

    if (yyin == NULL)
        fcc_fatal_errno("open_memstream()");

    /* TODO: Add -I list
                 -I$(pp_paths[0])
                 -I$(pp_paths[1])
                 -I$(pp_paths[2]) */
    snprintf(cmd, sizeof (cmd) - 1, "cpp %s", full_path);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
        fcc_fatal_errno("popen()");

    while (fgets(line, 512, fp))
        fwrite(line, strlen(line), 1, yyin);
    fflush(yyin);

    if (pclose(fp) < 0)
        fcc_fatal_errno("pclose()");
}

static void preprocess(const char *filename)
{
    char path[512] = {0};
    FILE *fp = NULL;

    vector_foreach(pp_paths, i) {
        const char *pp_path = vector_at(pp_paths, i);
        snprintf(path, sizeof (path) - 1, "%s/%s", pp_path, filename);
 
        printf("PP: Searching %s\n", path);
 
        fp = fopen(path, "rb");
    }

    if (fp == NULL)
        fcc_fatal_errno("fopen(%s)", filename);

    long size = ftell(fp);
    if (size < 0)
        fcc_fatal_errno("ftell()");

    fclose(fp);

    cpp(path, size);
}

tokens_t *pp(const char *filename)
{
    preprocess(filename);

    struct token *t = NULL;

    while ((t = peek_next())) {
        /* Skip directives of shape
           # 111 /path/... */
        if (t->type == T_HASH)
            while (t->type != T_NEWLINE)
                t = peek_next();

        vector_push_back(tokens, *t);
    }

    return &tokens;
}
