/* gen.c - Tests for code generator.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "back_end/gen.h"
#include "back_end/back_end.h"
#include "back_end/risc_v.h"
#include "util/io.h"
#include "utils/test_utils.h"

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

void __gen_test(const char *path, unused const char *filename, FILE *out_stream)
{
    struct ast_node *ast = gen_ast(path);
    ast_node_cleanup(ast);

    {
        char elf_path[256] = {0};
        snprintf(elf_path, sizeof (elf_path) - 1, "__elf.o");

        struct codegen_output output = {0};

        back_end_init(&output);

        back_end_emit_sym(&output, "fn_1", 0);
        back_end_emit_sym(&output, "fn_2", 4);
        back_end_emit_sym(&output, "fn_3", 8);

        back_end_native_addi(risc_v_reg_a7, risc_v_reg_zero, 93);
        back_end_native_addi(risc_v_reg_a0, risc_v_reg_zero, 123);
        back_end_native_syscall();

        back_end_emit(&output, elf_path);
    }

    char buf[8192 * 100];
    system_read(buf, "%s -D --section=.text __elf.o", __target_objdump);

    fputs(buf, out_stream);
}

int gen_test(const char *path, const char *filename)
{
    return compare_with_comment(path, filename, __gen_test);
}

int main()
{
    return do_on_each_file("gen", gen_test);
}