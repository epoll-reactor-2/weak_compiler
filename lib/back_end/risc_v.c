/* risc_v.c - RISC-V encoding.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "back_end/back_end.h"
#include "back_end/risc_v.h"

/**********************************************
 **         Generic instructions             **
 **********************************************/

static void write_uint32_le_m(void *addr, uint32_t v)
{
    uint8_t *mem = (uint8_t *) addr;
    mem[0] = (v      ) & 0xFF;
    mem[1] = (v >>  8) & 0xFF;
    mem[2] = (v >> 16) & 0xFF;
    mem[3] = (v >> 24) & 0xFF;
}

static bool risc_v_is_valid_imm(int32_t imm)
{
    return (((int32_t) (((uint32_t) imm) << 20)) >> 20) == imm;
}

static void risc_v_20_imm_op(uint32_t op, int32_t reg, int32_t imm)
{
    uint8_t code[4] = {0};
    write_uint32_le_m(code, op | (reg << 7) | (imm & 0xFFFFF000));
    put(code, 4);
}

/* Load [31:12] bits of the register from 20-bit imm, signextend & zero lower bits */
static void risc_v_lui(int32_t reg, int32_t imm)
{
    risc_v_20_imm_op(0x37, reg, imm);
}

/* Load PC + [31:12] imm to register */
static void risc_v_auipc(int32_t reg, int32_t imm)
{
    risc_v_20_imm_op(0x17, reg, imm);
}

static void risc_v_r_op(int op, int rds, int r1, int r2)
{
    uint8_t code[4] = {0};
    write_uint32_le_m(code, op | (rds << 7) | (r1 << 15) | (r2 << 20));
    put(code, 4);
}

/* I-type operation (sign-extended 12-bit immediate) */
static void risc_v_i_op_internal(int op, int rds, int r, uint32_t imm)
{
    uint8_t code[4];
    write_uint32_le_m(code, op | (rds << 7) | (r << 15) | (((uint32_t) imm) << 20));
    put(code, 4);
}

/* Set native register reg to sign-extended 32-bit imm */
static void risc_v_native_setreg32s(int reg, int imm)
{
    if (!risc_v_is_valid_imm(imm)) {
        /* Upper 20 bits aren't sign-extension. */
        if (imm & 0x800)
            /* Lower 12-bit part will sign-extend and subtract 0x1000 from LUI value */
            imm += 0x1000;
        risc_v_lui(reg, imm);
        if ((imm & 0xFFF) != 0) {
            risc_v_i_op_internal(risc_v_I_addi, reg, reg, imm & 0xFFF);
        }
    } else {
        risc_v_i_op_internal(risc_v_I_addi, reg, risc_v_reg_zero, imm & 0xFFF);
    }
}

static void risc_v_native_setreg32(int reg, int imm)
{
    risc_v_native_setreg32(reg, imm);
}

static int risc_v_i_to_r(int op)
{
    return op | 0x20;
}

static int risc_v_is_load_op(int op)
{
    return (op & 0xFF) == 0x03;
}

static void risc_v_i_op(int op, int rds, int r, uint32_t imm)
{
    if (risc_v_is_valid_imm(imm)) {
        risc_v_i_op_internal(op, rds, r, imm);
    } else if (!risc_v_is_load_op(op)) {
        if (op == risc_v_I_addi || op == risc_v_I_addiw && risc_v_is_valid_imm(imm >> 1)) {
            /* Lower to two consequent addi. */
            risc_v_i_op_internal(op, rds, r, imm >> 1);
            risc_v_i_op_internal(op, rds, rds, imm - (imm >> 1));
        } else {
            /* Reclaim register, load 32-bit imm, use in R-type op. */
            /* TODO: Claim regs. */
        }
    } else {

    }
}

void back_end_native_add(int dst, int reg1, int reg2)
{
    risc_v_r_op(risc_v_R_add, dst, reg1, reg2);
}

void back_end_native_addi(int dst, int reg1, int imm)
{
    risc_v_i_op(risc_v_I_addi, dst, reg1, imm);
}

void back_end_native_addiw(int dst, int reg1, int imm)
{
    risc_v_i_op(risc_v_I_addiw, dst, reg1, imm);
}

void back_end_native_sub(int dst, int reg1, int reg2)
{
    risc_v_r_op(risc_v_R_sub, dst, reg1, reg2);
}

void back_end_native_div(int dst, int reg1, int reg2)
{
    risc_v_r_op(risc_v_M_div, dst, reg1, reg2);
}

void back_end_native_mul(int dst, int reg1, int reg2)
{
    risc_v_r_op(risc_v_M_mul, dst, reg1, reg2);
}

void back_end_native_xor(int dst, int reg1, int reg2)
{
    risc_v_r_op(risc_v_R_xor, dst, reg1, reg2);
}

void back_end_native_and(int dst, int reg1, int reg2)
{
    risc_v_r_op(risc_v_R_and, dst, reg1, reg2);
}

void back_end_native_or(int dst, int reg1, int reg2)
{
    risc_v_r_op(risc_v_R_or, dst, reg1, reg2);
}

void back_end_native_sra(int dst, int reg1, int reg2)
{
    risc_v_r_op(risc_v_R_sra, dst, reg1, reg2);
}

void back_end_native_srl(int dst, int reg1, int reg2)
{
    risc_v_r_op(risc_v_R_srl, dst, reg1, reg2);
}

void back_end_native_lb(int dst, int addr, int off)
{
     risc_v_i_op(risc_v_I_lb, dst, addr, off);
}

void back_end_native_lbu(int dst, int addr, int off)
{
     risc_v_i_op(risc_v_I_lbu, dst, addr, off);
}

void back_end_native_lh(int dst, int addr, int off)
{
     risc_v_i_op(risc_v_I_lh, dst, addr, off);
}

void back_end_native_lhu(int dst, int addr, int off)
{
     risc_v_i_op(risc_v_I_lhu, dst, addr, off);
}

void back_end_native_lw(int dst, int addr, int off)
{
     risc_v_i_op(risc_v_I_lw, dst, addr, off);
}

void back_end_native_lwu(int dst, int addr, int off)
{
     risc_v_i_op(risc_v_I_lwu, dst, addr, off);
}

void back_end_native_ld(int dst, int addr, int off)
{
     risc_v_i_op(risc_v_I_ld, dst, addr, off);
}

void back_end_native_sb(int dst, int addr, int off)
{
    // put(risc_v_sb(dst, addr, off));
}

void back_end_native_sh(int dst, int addr, int off)
{
    // put(risc_v_sh(dst, addr, off));
}

void back_end_native_sw(int dst, int addr, int off)
{
    // put(risc_v_sw(dst, addr, off));
}

void back_end_native_sd(int dst, int addr, int off)
{
    // put(risc_v_sd(dst, addr, off));
}

void back_end_native_ret()
{
    risc_v_i_op(risc_v_I_jalr, risc_v_reg_zero, risc_v_reg_ra, 0);
}

void back_end_native_jmp_reg(int reg)
{
    // put(risc_v_jalr(risc_v_reg_zero, reg, 0));
}