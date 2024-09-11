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
    cfg_dir("elf", current_output_dir);
    char elf_path[256] = {0};
    char cmd[512] = {0};
    snprintf(elf_path, sizeof (elf_path) - 1, "%s/__elf.o", current_output_dir);

    struct codegen_output output = {0};

    hashmap_init(&output.fn_offsets, 512);

    uint8_t code[] = {
#if defined CONFIG_USE_BACKEND_X86_64
        0xb8, 0x3c, 0x00, 0x00, 0x00, /* mov    $0x3c,%eax */
        0xbf, 0x7b, 0x00, 0x00, 0x00, /* mov    $0x7b,%edi */
        0x0f, 0x05                    /* syscall           */
#elif defined CONFIG_USE_BACKEND_RISC_V
        0xf5, 0x48,                   /* li     a7,29      */
        0x01, 0x45,                   /* li     a0, 0      */
        0x73, 0x00                    /* ecall             */
#endif
    };

    static struct {
        const char *name;
        uint64_t    size;
    } sections[] = {
        { ".text",     sizeof (code) },
        { ".strtab",   100           },
        { ".shstrtab", 100           }
    };

    for (uint64_t i = 0; i < __weak_array_size(sections); ++i)
        elf_init_section(&output, sections[i].name, sections[i].size);

    instr_vector_t *instrs = elf_lookup_section(&output, ".text");

    for (uint64_t i = 0; i < __weak_array_size(code); ++i) {
        vector_push_back(*instrs, code[i]);
    }

    elf_init_symtab(&output, 10);


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