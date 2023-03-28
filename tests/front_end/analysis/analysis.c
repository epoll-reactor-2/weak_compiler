/* analysis.c - Test cases for all analyzers.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/analysis/analysis.h"
#include "front_end/ast/ast_node.h"
#include "front_end/lex/lex.h"
#include "front_end/parse/parse.h"
#include "utility/diagnostic.h"
#include "utils/test_utils.h"
#include <stdio.h>

extern FILE *yyin;
extern int yylex();
extern int yylex_destroy();

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

bool ignore_warns = false;

void(*analysis_fn)(ast_node_t *) = NULL;

bool analysis_test(const char *filename)
{
    bool success = true;
    char *err_buf = NULL;
    char *warn_buf = NULL;
    size_t err_buf_len = 0;
    size_t warn_buf_len = 0;

    diag_error_memstream = open_memstream(&err_buf, &err_buf_len);
    diag_warn_memstream = open_memstream(&warn_buf, &warn_buf_len);

    lex_reset_state();
    lex_init_state();

    yyin = fopen(filename, "r");
    if (yyin == NULL) {
        perror("fopen()");
        return false;
    }
    yylex();

    char   *msg = NULL;
    size_t  _ = 0;
    FILE   *msg_stream = open_memstream(&msg, &_);

    fseek(yyin, 0, SEEK_SET);
    extract_assertion_comment(yyin, msg_stream);

    tok_array_t *toks = lex_consumed_tokens();
    ast_node_t *ast = parse(toks->data, toks->data + toks->count);

    if (!setjmp(weak_fatal_error_buf)) {
        analysis_fn(ast);
        /// Normal code.
        if (!ignore_warns) {
            if (strcmp(warn_buf, msg) == 0) {
                printf("Success!\n");
                fflush(stdout);
            } else {
                printf("generated warning:\n%s", warn_buf);
                printf("expected warning:\n%s", msg);
                success = false;
                goto exit;
            }
        }
    } else {
        /// Code with fatal errors.
        if (strcmp(err_buf, msg) == 0) {
            printf("Success!\n");
            fflush(stdout);
        } else {
            printf("generated error:\n%s", err_buf);
            printf("expected error:\n%s", msg);
            success = false;
            goto exit;
        }
    }

    if (ignore_warns && !err_buf) {
        fprintf(stderr, "Expected compile error\n");
        fflush(stderr);
        success = false;
    }

exit:
    ast_node_cleanup(ast);
    yylex_destroy();

    return success;
}

int main()
{
    int ret = 0;

    analysis_fn = analysis_functions_analysis;
    ignore_warns = true;
    if (!do_on_each_file("/test_inputs/function_analysis", analysis_test)) {
        ret = -1;
        goto exit;
    }

    analysis_fn = analysis_variable_use_analysis;
    ignore_warns = true;
    if (!do_on_each_file("/test_inputs/variable_use_analysis/errors", analysis_test)) {
        ret = -1;
        goto exit;
    }

    analysis_fn = analysis_variable_use_analysis;
    ignore_warns = false;
    if (!do_on_each_file("/test_inputs/variable_use_analysis/warns", analysis_test)) {
        ret = -1;
        goto exit;
    }

    analysis_fn = analysis_type_analysis;
    ignore_warns = true;
    if (!do_on_each_file("/test_inputs/type_analysis", analysis_test)) {
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}