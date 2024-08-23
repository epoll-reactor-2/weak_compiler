#include "back_end/back_end.h"

/**********************************************
 **         Instruction encoding             **
 **********************************************/
/* R type */
#define risc_v_R_add                (0b110011 + (0 << 12))
#define risc_v_R_sub                (0b110011 + (0 << 12) + (0x20 << 25))
#define risc_v_R_xor                (0b110011 + (4 << 12))
#define risc_v_R_or                 (0b110011 + (6 << 12))
#define risc_v_R_and                (0b110011 + (7 << 12))
#define risc_v_R_sll                (0b110011 + (1 << 12))
#define risc_v_R_srl                (0b110011 + (5 << 12))
#define risc_v_R_sra                (0b110011 + (5 << 12) + (0x20 << 25))
#define risc_v_R_slt                (0b110011 + (2 << 12))
#define risc_v_R_sltu               (0b110011 + (3 << 12))
/* I type */
#define risc_v_I_addi               (0b0010011)
#define risc_v_I_xori               (0b0010011 + (4 << 12))
#define risc_v_I_ori                (0b0010011 + (6 << 12))
#define risc_v_I_andi               (0b0010011 + (7 << 12))
#define risc_v_I_slli               (0b0010011 + (1 << 12))
#define risc_v_I_srli               (0b0010011 + (5 << 12))
#define risc_v_I_srai               (0b0010011 + (5 << 12) + (0x20 << 25))
#define risc_v_I_slti               (0b0010011 + (2 << 12))
#define risc_v_I_sltiu              (0b0010011 + (3 << 12))
/* Load/store */
#define risc_v_I_lb                 (0b11                 )
#define risc_v_I_lh                 (0b11 + (1 << 12)     )
#define risc_v_I_lw                 (0b11 + (2 << 12)     )
#define risc_v_I_ld                 (0b11 + (3 << 12)     )
#define risc_v_I_lbu                (0b11 + (4 << 12)     )
#define risc_v_I_lhu                (0b11 + (5 << 12)     )
#define risc_v_S_sb                 (0b0100011            )
#define risc_v_S_sh                 (0b0100011 + (1 << 12))
#define risc_v_S_sw                 (0b0100011 + (2 << 12))
#define risc_v_S_sd                 (0b0100011 + (3 << 12))
/* Branches */
#define risc_v_B_beq                (0b1100011            )
#define risc_v_B_bne                (0b1100011 + (1 << 12))
#define risc_v_B_blt                (0b1100011 + (4 << 12))
#define risc_v_B_bge                (0b1100011 + (5 << 12))
#define risc_v_B_bltu               (0b1100011 + (6 << 12))
#define risc_v_B_bgeu               (0b1100011 + (7 << 12))
/* Jumps */
#define risc_v_I_jal                (0b1101111)
#define risc_v_I_jalr               (0b1100111)
/* Misc */
#define risc_v_I_lui                (0b0110111)
#define risc_v_I_auipc              (0b0010111)
#define risc_v_I_ecall              (0b1110011)
#define risc_v_I_ebreak             (0b1110011 + (1 << 20))
/* M */
#define risc_v_M_mul                (0b0110011 + (1 << 25))
#define risc_v_M_div                (0b0110011 + (1 << 25) + (4 << 12))
#define risc_v_M_mod                (0b0110011 + (1 << 25) + (6 << 12))

/* Registers */
#define risc_v_reg_zero              0
#define risc_v_reg_ra                1
#define risc_v_reg_sp                2
#define risc_v_reg_gp                3
#define risc_v_reg_tp                4
#define risc_v_reg_t0                5
#define risc_v_reg_t1                6
#define risc_v_reg_t2                7
#define risc_v_reg_s0                8
#define risc_v_reg_s1                9
#define risc_v_reg_a0               10
#define risc_v_reg_a1               11
#define risc_v_reg_a2               12
#define risc_v_reg_a3               13
#define risc_v_reg_a4               14
#define risc_v_reg_a5               15
#define risc_v_reg_a6               16
#define risc_v_reg_a7               17
#define risc_v_reg_s2               18
#define risc_v_reg_s3               19
#define risc_v_reg_s4               20
#define risc_v_reg_s5               21
#define risc_v_reg_s6               22
#define risc_v_reg_s7               23
#define risc_v_reg_s8               24
#define risc_v_reg_s9               25
#define risc_v_reg_s10              26
#define risc_v_reg_s11              27
#define risc_v_reg_t3               28
#define risc_v_reg_t4               29
#define risc_v_reg_t5               30
#define risc_v_reg_t6               31

static int risc_v_extract_bits(int imm, int i_start, int i_end, int d_start, int d_end)
{
    int v;

    if (d_end - d_start != i_end - i_start || i_start > i_end ||
        d_start > d_end)
        weak_fatal_error("Invalid bit copy");

    v = imm >> i_start;
    v = v & ((2 << (i_end - i_start)) - 1);
    v = v << d_start;
    return v;
}

static int risc_v_hi(int val)
{
    if ((val & (1 << 11)) != 0)
        return val + 4096;
    return val;
}

static int risc_v_lo(int val)
{
    if ((val & (1 << 11)) != 0)
        return (val & 0xFFF) - 4096;
    return val & 0xFFF;
}

static int risc_v_encode_R(int op, int rd, int rs1, int rs2)
{
    return op + (rd << 7) + (rs1 << 15) + (rs2 << 20);
}

static int risc_v_encode_I(int op, int rd, int rs1, int imm)
{
    if (imm > 2047 || imm < -2048)
        weak_fatal_error("Offset too large");

    if (imm < 0) {
        imm += 4096;
        imm &= (1 << 13) - 1;
    }
    return op + (rd << 7) + (rs1 << 15) + (imm << 20);
}

static int risc_v_encode_S(int op, int rs1, int rs2, int imm)
{
    if (imm > 2047 || imm < -2048)
        weak_fatal_error("Offset too large");

    if (imm < 0) {
        imm += 4096;
        imm &= (1 << 13) - 1;
    }
    return op + (rs1 << 15) + (rs2 << 20) +
           risc_v_extract_bits(imm, 0,  4,  7, 11) +
           risc_v_extract_bits(imm, 5, 11, 25, 31);
}

static int risc_v_encode_B(int op, int rs1, int rs2, int imm)
{
    int sign = 0;

    /* 13 signed bits, with bit zero ignored */
    if (imm > 4095 || imm < -4096)
        weak_fatal_error("Offset too large");

    if (imm < 0)
        sign = 1;

    return op + (sign << 31) + (rs1 << 15) + (rs2 << 20) +
           risc_v_extract_bits(imm, 11, 11,  7,  7) +
           risc_v_extract_bits(imm,  1,  4,  8, 11) +
           risc_v_extract_bits(imm,  5, 10, 25, 30);
}

static int risc_v_encode_J(int op, int rd, int imm)
{
    int sign = 0;

    if (imm < 0) {
        sign = 1;
        imm = -imm;
        imm = (1 << 21) - imm;
    }
    return op + (sign << 31) + (rd << 7) +
           risc_v_extract_bits(imm,  1, 10, 21, 30) +
           risc_v_extract_bits(imm, 11, 11, 20, 20) +
           risc_v_extract_bits(imm, 12, 19, 12, 19);
}

static int risc_v_encode_U(int op,  int rd,  int imm)
{
    return op + (rd << 7) + risc_v_extract_bits(imm, 12, 31, 12, 31);
}

#define risc_v_opcode(x, t) \
    static int risc_v_##x(int rd, int l, int r) { return risc_v_encode_##t(risc_v_##t##_##x, rd, l, r); }

#define risc_v_r_opcode(x) risc_v_opcode(x, R)
#define risc_v_i_opcode(x) risc_v_opcode(x, I)
#define risc_v_s_opcode(x) risc_v_opcode(x, S)
#define risc_v_b_opcode(x) risc_v_opcode(x, B)

risc_v_r_opcode(add)
risc_v_r_opcode(sub)
risc_v_r_opcode(or)
risc_v_r_opcode(xor)
risc_v_r_opcode(and)
risc_v_r_opcode(sll)
risc_v_r_opcode(srl)
risc_v_r_opcode(sra)
risc_v_r_opcode(slt)
risc_v_r_opcode(sltu)

risc_v_i_opcode(addi)
risc_v_i_opcode(xori)
risc_v_i_opcode(ori)
risc_v_i_opcode(andi)
risc_v_i_opcode(slli)
risc_v_i_opcode(srli)
risc_v_i_opcode(srai)
risc_v_i_opcode(slti)
risc_v_i_opcode(sltiu)
risc_v_i_opcode(lb)
risc_v_i_opcode(lh)
risc_v_i_opcode(lw)
risc_v_i_opcode(ld)
risc_v_i_opcode(lbu)
risc_v_i_opcode(lhu)
risc_v_i_opcode(jalr)

risc_v_s_opcode(sb)
risc_v_s_opcode(sh)
risc_v_s_opcode(sw)
risc_v_s_opcode(sd)

risc_v_b_opcode(beq)
risc_v_b_opcode(bne)
risc_v_b_opcode(blt)
risc_v_b_opcode(bge)
risc_v_b_opcode(bltu)
risc_v_b_opcode(bgeu)

/* Rest. */
static int risc_v_jal     (int rd,          int imm) { return risc_v_encode_J(risc_v_I_jal, rd, imm); }
static int risc_v_lui     (int rd,          int imm) { return risc_v_encode_U(risc_v_I_lui, rd, imm); }
static int risc_v_auipc   (int rd,          int imm) { return risc_v_encode_U(risc_v_I_auipc, rd, imm); }
static int risc_v_ecall   (                        ) { return risc_v_encode_I(risc_v_I_ecall,  risc_v_reg_zero, risc_v_reg_zero, 0); }
static int risc_v_ebreak  (                        ) { return risc_v_encode_I(risc_v_I_ebreak, risc_v_reg_zero, risc_v_reg_zero, 1); }
static int risc_v_nop     (                        ) { return risc_v_addi(risc_v_reg_zero, risc_v_reg_zero, 0); }
static int risc_v_mul     (int rd, int rs1, int rs2) { return risc_v_encode_R(risc_v_M_mul, rd, rs1, rs2); }
static int risc_v_div     (int rd, int rs1, int rs2) { return risc_v_encode_R(risc_v_M_div, rd, rs1, rs2); }
static int risc_v_mod     (int rd, int rs1, int rs2) { return risc_v_encode_R(risc_v_M_mod, rd, rs1, rs2); }
static int risc_v_ret     (                        ) { return risc_v_jalr(risc_v_reg_zero, risc_v_reg_ra, 0); }
static int risc_v_li      (int rd,          int imm) { return risc_v_addi(rd, risc_v_reg_zero, imm); }

/**********************************************
 **         Generic instructions             **
 **********************************************/

void back_end_native_add(int dst, int reg1, int reg2)
{
    put(risc_v_add(dst, reg1, reg2));
}

void back_end_native_addi(int dst, int reg1, int imm)
{
    put(risc_v_addi(dst, reg1, imm));
}

void back_end_native_sub(int dst, int reg1, int reg2)
{
    put(risc_v_sub(dst, reg1, reg2));
}

void back_end_native_div(int dst, int reg1, int reg2)
{
    put(risc_v_div(dst, reg1, reg2));
}

void back_end_native_mul(int dst, int reg1, int reg2)
{
    put(risc_v_mul(dst, reg1, reg2));
}

void back_end_native_ret()
{
    put(risc_v_ret());
}

void back_end_native_jmp_reg(int reg)
{
    put(risc_v_jalr(risc_v_reg_zero, reg, 0));
}