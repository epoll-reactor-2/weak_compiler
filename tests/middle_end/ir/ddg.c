/* ddg.c - Test case for data dependence graph.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/lex/lex.h"
#include "front_end/parse/parse.h"
#include "middle_end/ir/ddg.h"
#include "middle_end/ir/ir_dump.h"
#include "middle_end/ir/ir.h"
#include "util/diagnostic.h"
#include "utils/test_utils.h"
#include <stdio.h>

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

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

int ddg_test(const char *path, const char *filename)
{
    (void) filename;

    int     rc               = 0;
    char   *expected         = NULL;
    char   *generated        = NULL;
    size_t  _                = 0;
    FILE   * expected_stream = open_memstream(&expected, &_);
    FILE   *generated_stream = open_memstream(&generated, &_);

    if (!setjmp(weak_fatal_error_buf)) {
        struct ir_unit  ir = gen_ir(path);
        struct ir_node *it = ir.fn_decls;

        get_init_comment(yyin, expected_stream, NULL);

        while (it) {
            struct ir_fn_decl *decl = it->ir;

            ir_ddg_build(decl);
            ir_cfg_build(decl);
            ir_dump(generated_stream, decl);
            fprintf(generated_stream, "--------\n");
            ddg_dump(generated_stream, decl);
            it = it->next;
        }

        fflush(generated_stream);
        ir_unit_cleanup(&ir);

        if (strcmp(expected, generated) != 0) {
            printf("IR mismatch:\n%s\ngenerated\n%s\nexpected\n", generated, expected);
            rc = -1;
            goto exit;
        }
    } else {
        /* Error, will be printed in main. */
        rc = -1;
    }

exit:
    fclose(expected_stream);
    fclose(generated_stream);
    free(expected);
    free(generated);

    return rc;
}

int main()
{
    return do_on_each_file("ddg", ddg_test);
}