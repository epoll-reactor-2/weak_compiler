/* x86_64.c - Test cases for x86_64 codegen.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "back_end/x86_64.h"
#include "front_end/lex/lex.h"
#include "front_end/parse/parse.h"
#include "middle_end/ir/ir.h"
#include "middle_end/ir/ir_dump.h"
#include "middle_end/ir/type.h"
#include "middle_end/opt/opt.h"
#include "util/diagnostic.h"
#include "util/lexical.h"
#include "utils/test_utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

int x86_64_test(const char *path, unused const char *filename)
{
    char   *expected         = NULL;
    char   *generated        = NULL;
    size_t  _                = 0;
    FILE   *expected_stream  = open_memstream(&expected, &_);
    FILE   *generated_stream = open_memstream(&generated, &_);
    FILE   *code_stream      = fopen("/tmp/__code.S", "w");
    struct  ir_node    *it   = NULL;

    ftruncate(fileno(code_stream), 0);

    if (!setjmp(weak_fatal_error_buf)) {
        struct ir_unit unit = gen_ir(path);
        ir_type_pass(&unit);

        get_init_comment(yyin, expected_stream, NULL);

        /* ir_dump_unit(stdout, ir); */

        it = unit.fn_decls;
        while (it) {
            struct ir_fn_decl *decl = it->ir;
            /* Reordering before building CFG links. */
            ir_opt_reorder(decl);
            ir_opt_arith(decl);

            ir_cfg_build(decl);

            /* ir_dump(stdout, decl); */

            /* Wrong
               ir_opt_unreachable_code(decl); */

            /* There is some trouble with
               data flow of input function
               parameters.

               ir_opt_data_flow(decl); */

            /* ir_dump_cfg(tmp_cfg, decl); */
            /* ir_dump(stdout, decl); */
            /* cfg_edges_dump(stdout, decl); */
            it = it->next;
        }

        x86_64_gen(code_stream, &unit);
        ir_unit_cleanup(&unit);

        fflush(code_stream);

        system("cat /tmp/__code.S");
        system("nasm -f elf64 /tmp/__code.S -o /tmp/__code.o");
        system("ld /tmp/__code.o -o /tmp/__code");
        system("strace /tmp/__code");

    } else {
        /* Error, will be printed in main. */
        return -1;
    }

    fclose(code_stream);
    fclose(expected_stream);
    fclose(generated_stream);
    free(expected);
    free(generated);

    return 0;
}

int main()
{
    do_on_each_file("x86_64", x86_64_test);
    return -1;
}