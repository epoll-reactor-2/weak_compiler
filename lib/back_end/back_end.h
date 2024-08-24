/* back_end.h - Generic instructions.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_BACKEND_BACKEND_H
#define WEAK_COMPILER_BACKEND_BACKEND_H

#include "back_end/elf.h"

void back_end_init(struct codegen_output *output);

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
void back_end_native_jmp_reg(int reg);

#endif // WEAK_COMPILER_BACKEND_BACKEND_H