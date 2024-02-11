/* eval.c - Test cases for IR interpreter.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "back_end/eval.h"
#include "front_end/lex/lex.h"
#include "front_end/parse/parse.h"
#include "middle_end/ir/ir.h"
#include "middle_end/ir/ir_dump.h"
#include "middle_end/ir/type.h"
#include "middle_end/opt/opt.h"
#include "util/diagnostic.h"
#include "utils/test_utils.h"
#include <stdio.h>

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

void __eval_test(const char *path, const char *filename, FILE *out_stream)
{
    (void) filename;

    struct ir_unit  ir = gen_ir(path);
    struct ir_node *it = ir.fn_decls;
    ir_type_pass(&ir);

    while (it) {
        struct ir_fn_decl *decl = it->ir;
        /* Reordering before building CFG links. */
        ir_opt_reorder(decl);
        ir_opt_arith(decl);
        ir_cfg_build(decl);

        /* Wrong
            ir_opt_unreachable_code(decl); */
        it = it->next;
    }

    int32_t exit_code = eval(&ir);
    fprintf(out_stream, "%d\n", exit_code);

    ir_unit_cleanup(&ir);
}

int eval_test(const char *path, const char *filename)
{
    (void) filename;

    return compare_with_comment(path, filename, __eval_test);
}

int main()
{
    return do_on_each_file("eval", eval_test);
}
