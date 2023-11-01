/* ssa.c - Tests for SSA form.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/lex/lex.h"
#include "middle_end/ir/dump.h"
#include "middle_end/ir/ssa.h"
#include "util/diagnostic.h"
#include "utils/test_utils.h"
#include <stdio.h>

extern int yylex_destroy();

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

char current_output_dir[128];

void idom_dump(FILE *stream, struct ir_func_decl *decl)
{
    struct ir_node *it = decl->body;

    while (it) {
        if (it->idom)
            fprintf(
                stream, "idom(%ld) = %ld",
                it->instr_idx,
                it->idom->instr_idx
            );

        if (it->df.count > 0) {
            fprintf(stream, ", ");
            ir_dump_dominance_frontier(stream, it);
        } else
            fprintf(stream, "\n");

        it = it->next;
    }
}

bool ir_test(const char *path, const char *filename)
{
    bool    ok                 = 1;
    char   *expected           = NULL;
    char   *generated          = NULL;
    size_t  _                  =  0;
    FILE   *expected_stream    = open_memstream(&expected, &_);
    FILE   *generated_stream   = open_memstream(&generated, &_);
    char    dom_path[256]      = {0};

    snprintf(dom_path, 255, "%s/%s_dom_tree.dot", current_output_dir, filename);

    FILE   *dom_stream         = fopen(dom_path, "w");

    if (!setjmp(weak_fatal_error_buf)) {
        struct ir_unit *ir = gen_ir(path);
        struct ir_node *it = ir->func_decls;

        extract_assertion_comment(yyin, expected_stream);

        while (it) {
            ir_dominator_tree(it->ir);
            ir_dominance_frontier(it->ir);
            ir_dump_dom_tree(dom_stream, it->ir);
            ir_dump(generated_stream, it->ir);
            fprintf(generated_stream, "--------\n");
            idom_dump(generated_stream, it->ir);
            fflush(generated_stream);
            it = it->next;
        }

        ir_unit_cleanup(ir);

        if (strcmp(expected, generated) != 0) {
            printf("IR mismatch:\n%s\ngot,\n%s\nexpected\n", generated, expected);
            fflush(stdout);
            ok = 0;
            goto exit;
        }
        printf("Success!\n");
    } else {
        /// Error, will be printed in main.
        ok = 0;
    }

exit:
    yylex_destroy();
    fclose(expected_stream);
    fclose(generated_stream);
    fclose(dom_stream);
    free(expected);
    free(generated);

    return ok;
}

char *err_buf       = NULL;
char *warn_buf      = NULL;
size_t err_buf_len  = 0;
size_t warn_buf_len = 0;

int run()
{
    int   ret  = 0;
    char *path = "/test_inputs/dom";

    cfg_dir("dom", current_output_dir);

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

    if (run() < 0)
        return -1;

    fclose(diag_error_memstream);
    fclose(diag_warn_memstream);
    free(err_buf);
    free(warn_buf);
    return 0;
}