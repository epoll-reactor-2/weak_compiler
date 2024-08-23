#ifndef WEAK_COMPILER_BACKEND_BACKEND_H
#define WEAK_COMPILER_BACKEND_BACKEND_H

#include "back_end/elf.h"

void back_end_init(struct codegen_output *output);

void put(int code);

void back_end_native_add    (int dst, int reg1, int reg2);
void back_end_native_addi   (int dst, int reg1, int imm);
void back_end_native_sub    (int dst, int reg1, int reg2);
void back_end_native_div    (int dst, int reg1, int reg2);
void back_end_native_mul    (int dst, int reg1, int reg2);
void back_end_native_ret    ();
void back_end_native_jmp_reg(int reg);

#endif // WEAK_COMPILER_BACKEND_BACKEND_H