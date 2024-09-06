/* back_end.c - Tests for backend interface.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */


#include "back_end/back_end.h"
#include "utils/test_utils.h"
#include <stddef.h>
#include <stdio.h>

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

char current_output_dir[128];

int main()
{
    return 0;
    cfg_dir("back_end", current_output_dir);
    char elf_path[256] = {0};
    char cmd[512] = {0};
    snprintf(elf_path, sizeof (elf_path) - 1, "%s/__elf.o", current_output_dir);

    struct codegen_output output = {0};

    hashmap_init(&output.fn_offsets, 512);
    back_end_init(&output);

    struct elf_entry elf = {
        .filename = elf_path,
        .output   = output
    };

    elf_init(&elf);
    elf_exit();

#if defined CONFIG_USE_BACKEND_RISC_V
    snprintf(cmd, sizeof (cmd) - 1, "riscv64-linux-gnu-readelf -a %s", elf_path);
#elif defined CONFIG_USE_BACKEND_X86_64
    snprintf(cmd, sizeof (cmd) - 1, "readelf -a %s", elf_path);
#endif
    system(cmd);

#if defined CONFIG_USE_BACKEND_RISC_V
    snprintf(cmd, sizeof (cmd) - 1, "riscv64-linux-gnu-objdump -D %s", elf_path);
#elif defined CONFIG_USE_BACKEND_X86_64
    snprintf(cmd, sizeof (cmd) - 1, "objdump -D %s", elf_path);
#endif
    system(cmd);
}