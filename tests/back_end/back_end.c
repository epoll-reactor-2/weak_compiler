/* back_end.c - Test cases for back end interface.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */
#include "back_end/elf.h"
#include "back_end/back_end.h"
#include "back_end/risc_v.h"
#include "utils/test_utils.h"

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

char current_output_dir[128];

void run(const char *path)
{
    char cmd[512] = {0};

    snprintf(cmd, sizeof (cmd) - 1, "%s -a %s", __target_readelf, path);
    system(cmd);

    snprintf(cmd, sizeof (cmd) - 1, "%s -D %s", __target_objdump, path);
    system(cmd);

    snprintf(cmd, sizeof (cmd) - 1, "%s %s", __target_exec, path);
    system(cmd);

    printf("*** RISC-V file exited with code %d\n\n", WEXITSTATUS(system(cmd)));
}

int main()
{
    cfg_dir("elf", current_output_dir);
    char elf_path[256] = {0};
    snprintf(elf_path, sizeof (elf_path) - 1, "%s/__elf.o", current_output_dir);

    struct codegen_output output = {0};

    back_end_init(&output);

    back_end_emit_sym(&output, "fn_1", 0);
    back_end_emit_sym(&output, "fn_2", 4);
    back_end_emit_sym(&output, "fn_3", 8);

    back_end_native_addi(risc_v_reg_a7, risc_v_reg_zero, 93);
    back_end_native_addi(risc_v_reg_a0, risc_v_reg_zero, 123);
    back_end_native_syscall();

    back_end_emit(&output, elf_path);

    run(elf_path);

    return 0;
}