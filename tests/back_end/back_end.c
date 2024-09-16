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

uint64_t calculate_strtab_size(struct elf_symtab_entry *e, uint64_t size)
{
    uint64_t len = 0;

    for (uint64_t i = 0; i < size; ++i)
        len += strlen(e[i].name) + 1;

    return len;
}

int main()
{
    cfg_dir("elf", current_output_dir);
    char elf_path[256] = {0};
    char cmd[512] = {0};
    snprintf(elf_path, sizeof (elf_path) - 1, "%s/__elf.o", current_output_dir);

    struct codegen_output output = {0};

    hashmap_init(&output.fn_offsets, 512);

    struct elf_symtab_entry symtab[] = {
        { "fn_1",   0 },
        { "fn_2",  12 },
    };
    uint64_t strtab_len = calculate_strtab_size(symtab, __weak_array_size(symtab));

    struct {
        const char *name;
        uint64_t    size;
    } sections[] = {
        { ".text",     12            },
        { ".strtab",   strtab_len    },
        { ".shstrtab", 100           },
    };

    for (uint64_t i = 0; i < __weak_array_size(sections); ++i)
        elf_init_section(&output, sections[i].name, sections[i].size);

    for (uint64_t i = 0; i < __weak_array_size(symtab); ++i)
        vector_push_back(output.symtab, symtab[i]);

    elf_init_symtab(&output, __weak_array_size(symtab));

    back_end_init(&output);
    back_end_native_addi(risc_v_reg_a7, risc_v_reg_zero, 93);
    back_end_native_addi(risc_v_reg_a0, risc_v_reg_zero, 123);
    back_end_native_syscall();

    struct elf_entry elf = {
        .filename = elf_path,
        .output   = output
    };

    elf_init(&elf);
    elf_exit(&elf);

    snprintf(cmd, sizeof (cmd) - 1, "%s -a %s", __target_readelf, elf_path);
    system(cmd);

    snprintf(cmd, sizeof (cmd) - 1, "%s -D %s", __target_objdump, elf_path);
    system(cmd);

    snprintf(cmd, sizeof (cmd) - 1, "%s %s", __target_exec, elf_path);
    system(cmd);

    printf("*** RISC-V file exited with code %d\n\n", WEXITSTATUS(system(cmd)));

    return -1;
}