/* elf.c - Test cases for ELF header generator.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "back_end/elf.h"
#include "util/unreachable.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

int main()
{
    struct elf_entry elf = {
        .arch     = ARCH_RISC_V,
        .filename = "__elf.o"
    };

    elf_init(&elf);

    /*
    0x1141    addi    sp,sp,-16
    0xe422    sd      s0,8(sp)
    0x4781    li      a5,0
    0x853e    mv      a0,a5
    0x6422    ld      s0,8(sp)
    0x0141    addi    sp,sp,16
    0x8082    ret
    */
    unused uint8_t risc_v_code[] = {
        0x41, 0x11, 0x22, 0xe4,
        0x81, 0x47,
        0x3e, 0x85, 0x22, 0x64,
        0x41, 0x01, 0x82, 0x80,
    };
    unused uint8_t x86_64_code[] = {
        0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00, /* mov $0x3c, %rax */
        0x48, 0x31, 0xff,                         /* xor %rdi, %rdi */
        0x0f, 0x05                                /* syscall */
    };
    elf_put_code(risc_v_code, sizeof (risc_v_code));
    elf_exit();

    system("riscv64-linux-gnu-readelf -a __elf.o");
    system("riscv64-linux-gnu-objdump -D __elf.o");
}
