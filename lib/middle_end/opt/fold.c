/* fold.c - Constant folding.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "middle_end/ir/ir.h"
#include "middle_end/ir/dump.h"
#include "middle_end/opt/opt.h"
#include "util/compiler.h"
#include "util/hashmap.h"
#include "util/vector.h"
#include "util/unreachable.h"
#include <string.h>
#include <assert.h>

static struct ir_node last_folded;
static hashmap_t      consts_mapping;

static void consts_mapping_init()
{
    if (consts_mapping.buckets) {
        hashmap_destroy(&consts_mapping);
    }
    hashmap_init(&consts_mapping, 512);
}


__weak_really_inline static void consts_mapping_add(uint64_t idx, uint64_t value)
{
    hashmap_put(&consts_mapping, idx, value);
    printf("Consts mapping: add idx:%ld, value:%ld\n", idx, value);
}

__weak_really_inline static void consts_mapping_remove(uint64_t idx)
{
    printf("Consts mapping: remove idx:%ld\n", idx);
    hashmap_remove(&consts_mapping, idx);
}

__weak_really_inline static union ir_imm_val consts_mapping_get(uint64_t idx)
{
    uint64_t got = hashmap_get(&consts_mapping, idx);
    printf("Consts mapping: get value of idx:%ld -> %ld\n", idx, got);
    return (union ir_imm_val) (int32_t) got;
}

__weak_really_inline static void consts_mapping_update(uint64_t idx, uint64_t value)
{   
    hashmap_remove(&consts_mapping, idx);
    hashmap_put(&consts_mapping, idx, value);
    printf("Consts mapping: update idx:%ld, value:%ld\n", idx, value);
}

__weak_really_inline static bool consts_mapping_is_const(uint64_t idx)
{
    uint64_t got = hashmap_get(&consts_mapping, idx);
    printf("Consts mapping: is_const? idx:%ld -> %s\n", idx, (got != 0) ? "yes" : "no");
    return got != 0;
}

__weak_really_inline static bool fold_booleans(enum token_type op, bool l, bool r)
{
    switch (op) {
    case TOK_BIT_AND: return l & r;
    case TOK_BIT_OR:  return l | r;
    case TOK_XOR:     return l ^ r;
    default:
        weak_unreachable("Something went wrong.");
    }
}

__weak_really_inline static int32_t fold_ints(enum token_type op, int32_t l, int32_t r)
{
    switch (op) {
    case TOK_AND:     return l && r;
    case TOK_OR:      return l || r;
    case TOK_XOR:     return l  ^ r;
    case TOK_BIT_AND: return l  & r;
    case TOK_BIT_OR:  return l  | r;
    case TOK_EQ:      return l == r;
    case TOK_NEQ:     return l != r;
    case TOK_GT:      return l  > r;
    case TOK_LT:      return l  < r;
    case TOK_GE:      return l >= r;
    case TOK_LE:      return l <= r;
    case TOK_SHL:     return l << r;
    case TOK_SHR:     return l >> r;
    case TOK_PLUS:    return l  + r;
    case TOK_MINUS:   return l  - r;
    case TOK_STAR:    return l  * r;
    case TOK_SLASH:   return l  / r;
    case TOK_MOD:     return l  % r;
    default:
        weak_unreachable("Something went wrong.");
    }
}

__weak_really_inline static float fold_floats(enum token_type op, float l, float r)
{
    switch (op) {
    case TOK_EQ:      return l == r;
    case TOK_NEQ:     return l != r;
    case TOK_GT:      return l  > r;
    case TOK_LT:      return l  < r;
    case TOK_GE:      return l >= r;
    case TOK_LE:      return l <= r;
    case TOK_PLUS:    return l  + r;
    case TOK_MINUS:   return l  - r;
    case TOK_STAR:    return l  * r;
    case TOK_SLASH:   return l  / r;
    default:
        weak_unreachable("Something went wrong.");
    }
}

__weak_wur __weak_really_inline static struct ir_node fold_imm(
    enum  token_type  op,
    enum  ir_imm_type type,
    union ir_imm_val  lhs,
    union ir_imm_val  rhs
) {
    switch (type) {
    case IMM_BOOL:  return ir_imm_bool_init (fold_booleans(op, lhs.__bool,  rhs.__bool));
    case IMM_CHAR:  return ir_imm_char_init (fold_ints    (op, lhs.__char,  rhs.__char));
    case IMM_FLOAT: return ir_imm_float_init(fold_floats  (op, lhs.__float, rhs.__float));
    case IMM_INT:   return ir_imm_int_init  (fold_ints    (op, lhs.__int,   rhs.__int));
    default:
        weak_unreachable("Something went wrong.");
    }
}

static void fold_node(struct ir_node *ir);

__weak_really_inline static void fold_store(struct ir_store *ir)
{
    switch (ir->type) {
    case IR_STORE_BIN: {
        struct ir_bin *bin = ir->body.ir;

        if (bin->lhs.type == IR_IMM &&
            bin->rhs.type == IR_IMM) {
            fold_node(&ir->body);
            ir_node_cleanup(ir->body);
            ir->body = last_folded;
            ir->type = IR_STORE_IMM;

            struct ir_imm *imm = ir->body.ir;
            consts_mapping_add(ir->idx, imm->imm.__int);
        }

        if ((bin->lhs.type == IR_SYM &&
             bin->rhs.type == IR_IMM
            ) || (
             bin->lhs.type == IR_SYM &&
             bin->rhs.type == IR_SYM
            )
        ) {
            struct ir_sym *sym = bin->lhs.ir;

            if (consts_mapping_is_const(sym->idx)) {
                fold_node(&ir->body);
                ir_node_cleanup(ir->body);
                ir->body = last_folded;
                ir->type = IR_STORE_IMM;

                struct ir_imm *imm = ir->body.ir;
                consts_mapping_update(ir->idx, imm->imm.__int);
            }
        }

        break;
    }
    case IR_STORE_VAR: {
        struct ir_sym *sym = ir->body.ir;

        if (consts_mapping_is_const(sym->idx)) {
            union ir_imm_val imm = consts_mapping_get(sym->idx);

            ir_node_cleanup(ir->body);
            /// \todo: Any type
            ir->body = ir_imm_int_init(imm.__int);
            ir->type = IR_STORE_IMM;

            consts_mapping_update(ir->idx, imm.__int);
        } else {
            consts_mapping_remove(ir->idx);
        }
        break;
    }
    case IR_STORE_IMM: {
        struct ir_imm *imm = ir->body.ir;

        if (consts_mapping_is_const(ir->idx)) {
            consts_mapping_update(ir->idx, imm->imm.__int);
        }
    }
    default:
        break;
    }
}

__weak_really_inline static void fold_bin(struct ir_bin *ir)
{
    if (ir->lhs.type == IR_IMM &&
        ir->rhs.type == IR_IMM) {
        struct ir_imm *l_imm = ir->lhs.ir;
        struct ir_imm *r_imm = ir->rhs.ir;
        assert(l_imm->type == r_imm->type);

        last_folded = fold_imm(ir->op, l_imm->type, l_imm->imm, r_imm->imm);
    }

    if (ir->lhs.type == IR_SYM &&
        ir->rhs.type == IR_IMM) {
        struct ir_sym *sym = ir->lhs.ir;
        struct ir_imm *imm = ir->rhs.ir;

        if (consts_mapping_is_const(sym->idx)) {
            union ir_imm_val imm_val = consts_mapping_get(sym->idx);

            last_folded = fold_imm(ir->op, imm->type, imm_val, imm->imm);
        }
    }

    if (ir->lhs.type == IR_SYM &&
        ir->rhs.type == IR_SYM) {
        struct ir_sym *lhs_sym = ir->lhs.ir;
        struct ir_sym *rhs_sym = ir->rhs.ir;

        bool lhs_const = consts_mapping_is_const(lhs_sym->idx);
        bool rhs_const = consts_mapping_is_const(rhs_sym->idx);

        if (lhs_const && rhs_const) {
            union ir_imm_val lhs_imm = consts_mapping_get(lhs_sym->idx);
            union ir_imm_val rhs_imm = consts_mapping_get(rhs_sym->idx);

            last_folded = fold_imm(ir->op, IMM_INT, lhs_imm, rhs_imm);
        }
    }
}

__weak_really_inline static void fold_ret(struct ir_ret *ir)
{
    if (ir->is_void) return;

    struct ir_sym *sym = ir->op.ir;

    if (consts_mapping_is_const(sym->idx)) {
        union ir_imm_val imm = consts_mapping_get(sym->idx);

        /// \todo: Immediate emit for all types.
        ir_node_cleanup(ir->op);
        ir->op = ir_imm_int_init(imm.__int);
    }
}

static void fold_node(struct ir_node *ir)
{
    switch (ir->type) {
    case IR_ALLOCA:
    case IR_IMM:
    case IR_SYM:
    case IR_LABEL:
    case IR_JUMP:
    case IR_COND:
    case IR_MEMBER:
    case IR_ARRAY_ACCESS:
    case IR_TYPE_DECL:
    case IR_FUNC_DECL:
    case IR_FUNC_CALL:
        break;
    case IR_STORE: {
        fold_store(ir->ir);
        break;
    }
    case IR_BIN: {
        fold_bin(ir->ir);
        break;
    }
    case IR_RET:
    case IR_RET_VOID: {
        fold_ret(ir->ir);
        break;
    }
    default:
        weak_unreachable("Something went wrong.");
    }
}

static void fold_remove_unused_stmts(struct ir_func_decl *decl)
{
    /// Vector used for convenient API.
    vector_t(struct ir_node) stmts;

    stmts.data  = decl->body;
    stmts.count = decl->body_size;
    stmts.size  = stmts.count * sizeof (struct ir_node *);

    vector_foreach_back(stmts, i) {
        struct ir_node *node = &vector_at(stmts, i);

        switch (node->type) {

        case IR_ALLOCA: {
            struct ir_alloca *alloca = node->ir;

            if (consts_mapping_is_const(alloca->idx)) {
                vector_erase(stmts, (size_t) node->instr_idx);
            }
            break;
        }

        case IR_STORE: {
            struct ir_store *store = node->ir;

            if (consts_mapping_is_const(store->idx)) {
                vector_erase(stmts, (size_t) node->instr_idx);
            }
            break;
        }

        default:
            break;
        }
    }

    decl->body_size = stmts.count;
}

static void fold(struct ir_func_decl *decl)
{
    for (uint64_t i = 0; i < decl->body_size; ++i) {
        struct ir_node *node = &decl->body[i];
        fold_node(node);
    }

    fold_remove_unused_stmts(decl);
}

void ir_opt_fold(struct ir *ir)
{
    consts_mapping_init();

    for (uint64_t i = 0; i < ir->decls_size; ++i) {
        struct ir_func_decl *decl = ir->decls[i].ir;
        fold(decl);
    }
}