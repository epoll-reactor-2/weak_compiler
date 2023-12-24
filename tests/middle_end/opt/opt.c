/* opt.c - Test cases for optimizations.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/lex/lex.h"
#include "middle_end/ir/ddg.h"
#include "middle_end/ir/dump.h"
#include "middle_end/opt/opt.h"
#include "util/diagnostic.h"
#include "utils/test_utils.h"
#include <stdio.h>
#include <sys/stat.h>

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

void (*opt_fn)(struct ir_fn_decl *);

char current_output_dir[128];

void ddg_dump(FILE *stream, struct ir_fn_decl *decl)
{
    struct ir_node *it = decl->body;

    while (it) {
        ir_vector_t *ddgs = &it->ddg_stmts;
        fprintf(stream, "instr %2ld: depends on (", it->instr_idx);
        vector_foreach(*ddgs, i) {
            struct ir_node *stmt = vector_at(*ddgs, i);
            fprintf(stream, "%ld", stmt->instr_idx);
            if (i < ddgs->count - 1)
                fprintf(stream, ", ");
        }
        fprintf(stream, ")\n");
        it = it->next;
    }
}

bool ir_test(const char *path, const char *filename)
{
    bool    ok                   = 1;
    char   *expected             = NULL;
    char   *generated            = NULL;
    size_t  _                    = 0;
    char    before_opt_path[256] = {0};
    char     after_opt_path[256] = {0};

    snprintf(before_opt_path, 255, "%s/%s.dot", current_output_dir, filename);
    snprintf( after_opt_path, 255, "%s/%s_optimized.dot", current_output_dir, filename);

    FILE   * expected_stream = open_memstream(&expected, &_);
    FILE   *generated_stream = open_memstream(&generated, &_);
    FILE   *before_opt_stream = fopen(before_opt_path, "w");
    FILE   * after_opt_stream = fopen( after_opt_path, "w");

    struct ir_unit *ir = NULL;

    if (!setjmp(weak_fatal_error_buf)) {
        ir = gen_ir(path);
        struct ir_node *it = ir->fn_decls;

        extract_assertion_comment(yyin, expected_stream);

        while (it) {
            struct ir_fn_decl *decl = it->ir;
            ir_cfg_build(decl);
            ir_ddg_build(decl);
            opt_fn(decl);

            ir_dump_cfg(after_opt_stream, decl);
            ir_dump(generated_stream, decl);

            it = it->next;
        }

        fflush(generated_stream);

        if (strcmp(expected, generated) != 0) {
            ir_dump_unit(stdout, ir);
            printf("IR mismatch:\n%s\ngenerated\n%s\nexpected\n", generated, expected);
            ok = 0;
            goto exit;
        }
        printf("Success!\n");
    } else {
        /* Error, will be printed in main. */
        ok = 0;
    }

exit:
    yylex_destroy();
    fclose(expected_stream);
    fclose(generated_stream);
    fclose(before_opt_stream);
    fclose( after_opt_stream);
    free(expected);
    free(generated);
    ir_unit_cleanup(ir);

    return ok;
}

char *err_buf       = NULL;
char *warn_buf      = NULL;
size_t err_buf_len  = 0;
size_t warn_buf_len = 0;

int run(const char *dir)
{
    int  ret       =  0;
    char path[256] = {0};
    snprintf(path, sizeof (path) - 1, "/test_inputs/%s", dir);

    cfg_dir(dir, current_output_dir);

    if (!do_on_each_file(path, ir_test)) {
        ret = -1;

        if (err_buf)
            fputs(err_buf, stderr);

        if (warn_buf)
            fputs(warn_buf, stderr);
    }

    return ret;
}

int main()
{
    diag_error_memstream = open_memstream(&err_buf, &err_buf_len);
    diag_warn_memstream = open_memstream(&warn_buf, &warn_buf_len);

#if 0
    opt_fn = ir_opt_fold;
    if (run("fold") < 0)
        return -1;
#endif

#if 0
    opt_fn = ir_opt_arith;
    if (run("arith") < 0)
        return -1;
#endif

#if 0
    opt_fn = ir_opt_dead_code_elimination;
    if (run("dead_code") < 0)
        return -1;
#endif

#if 0
    opt_fn = ir_opt_reorder;
    if (run("reorder") < 0)
        return -1;
#endif

#if 0
    opt_fn = ir_opt_unreachable_code;
    if (run("unreachable") < 0)
        return -1;
#endif

#if 1
    opt_fn = ir_opt_data_flow;
    if (run("data_flow") < 0)
        return -1;
#endif

    fclose(diag_error_memstream);
    fclose(diag_warn_memstream);
    free(err_buf);
    free(warn_buf);
    return 0;
}