/* back_end.h - Generic instructions.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_BACKEND_BACKEND_H
#define WEAK_COMPILER_BACKEND_BACKEND_H

#include "back_end/elf.h"

void back_end_init(struct codegen_output *output);
void back_end_emit(struct codegen_output *output, const char *path);
void back_end_emit_sym(const char *name, uint64_t off);

/* Returns number of generated bytes
   at this point of time. */
uint64_t back_end_seek();
/* Used to set position from which we want
   to insert code with `back_end_native_*`
   functions. */
void back_end_seek_set(uint64_t seek);

void put(uint8_t *code, uint64_t size);

void back_end_native_add    (int dst, int reg1, int reg2);
void back_end_native_addi   (int dst, int reg1, int imm);
void back_end_native_addiw  (int dst, int reg1, int imm);
void back_end_native_sub    (int dst, int reg1, int reg2);
void back_end_native_div    (int dst, int reg1, int reg2);
void back_end_native_mul    (int dst, int reg1, int reg2);
void back_end_native_xor    (int dst, int reg1, int reg2);
void back_end_native_xori   (int dst, int reg1, int imm);
void back_end_native_and    (int dst, int reg1, int reg2);
void back_end_native_or     (int dst, int reg1, int reg2);
void back_end_native_sra    (int dst, int reg1, int reg2);
void back_end_native_srl    (int dst, int reg1, int reg2);

void back_end_native_li     (int dst,           int imm);
void back_end_native_lb     (int dst, int addr, int off);
void back_end_native_lbu    (int dst, int addr, int off);
void back_end_native_lh     (int dst, int addr, int off);
void back_end_native_lhu    (int dst, int addr, int off);
void back_end_native_lw     (int dst, int addr, int off);
void back_end_native_lwu    (int dst, int addr, int off);
void back_end_native_ld     (int dst, int addr, int off);

void back_end_native_sb     (int dst, int addr, int off);
void back_end_native_sh     (int dst, int addr, int off);
void back_end_native_sw     (int dst, int addr, int off);
void back_end_native_sd     (int dst, int addr, int off);

void back_end_native_ret    ();
void back_end_native_call   (int off);
void back_end_native_jmp_reg(int reg);

void back_end_native_syscall_0(int syscall);
void back_end_native_syscall_1(int syscall, int _1);
void back_end_native_syscall_2(int syscall, int _1, int _2);
void back_end_native_syscall_3(int syscall, int _1, int _2, int _3);
void back_end_native_syscall_4(int syscall, int _1, int _2, int _3, int _4);
void back_end_native_syscall_5(int syscall, int _1, int _2, int _3, int _4, int _5);
void back_end_native_syscall_6(int syscall, int _1, int _2, int _3, int _4, int _5, int _6);

void back_end_native_prologue(int stack_usage);
void back_end_native_epilogue(int stack_usage);

#endif // WEAK_COMPILER_BACKEND_BACKEND_H