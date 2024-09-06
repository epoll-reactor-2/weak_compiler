/* elf.c - Test cases for ELF generator.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */
#include "back_end/elf.h"
#include "utils/test_utils.h"

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

char current_output_dir[128];

int main()
{
    cfg_dir("back_end", current_output_dir);
    char elf_path[256] = {0};
    char cmd[512] = {0};
    snprintf(elf_path, sizeof (elf_path) - 1, "%s/__elf.o", current_output_dir);

    struct codegen_output output = {0};

    hashmap_init(&output.fn_offsets, 512);

    /* TODO: Don't work as should if we change order
             of sections. */
    static const char *sections[] = {
        ".text",
        ".data",
        ".rodata",
        ".strtab",
        ".shstrtab",
        ".init_array",
        ".fini_array",
        ".ctors",
        ".dtors",
    };

    for (uint64_t i = 0; i < __weak_array_size(sections); ++i)
        elf_init_section(&output, sections[i], 100);

    elf_init_section(&output, ".symtab", /* ELF symtab entry size. */ 24 * 2);

    instr_vector_t *instrs = elf_lookup_section(&output, ".text");

    // for (uint64_t i = 0; i < 10; ++i)
        // vector_push_back(*instrs, 0xff);

    struct elf_entry elf = {
        .filename = elf_path,
        .output   = output
    };

    elf_init(&elf);
    elf_exit(&elf);

#if defined CONFIG_USE_BACKEND_RISC_V
    snprintf(cmd, sizeof (cmd) - 1, "riscv64-linux-gnu-readelf -a %s", elf_path);
#elif defined CONFIG_USE_BACKEND_X86_64
    snprintf(cmd, sizeof (cmd) - 1, "readelf -a %s", elf_path);
#endif
    system(cmd);

    return -1;
}