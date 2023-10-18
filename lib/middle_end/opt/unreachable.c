/* unreachable.c - Unreachable code remove.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "middle_end/opt/opt.h"
#include "middle_end/ir/ir.h"
#include "middle_end/ir/dump.h"
#include "util/hashmap.h"

__weak_really_inline static void mark_visited(bool *visited, struct ir_node *ir)
{
    visited[ir->instr_idx] = 1;
}

static void traverse(bool *visited, int32_t *max_id, struct ir_node *ir)
{
    if (visited[ir->instr_idx]) return;

    if (*max_id < ir->instr_idx)
        *max_id = ir->instr_idx;

    switch (ir->type) {
    case IR_IMM:
    case IR_SYM:
    case IR_BIN:
    case IR_MEMBER:
    case IR_STORE:
    case IR_ALLOCA:
    case IR_ALLOCA_ARRAY:
    case IR_FUNC_CALL: {
        mark_visited(visited, ir);
        traverse(visited, max_id, ir->next);
        break;
    }
    case IR_JUMP: {
        struct ir_jump *jmp = ir->ir;
        mark_visited(visited, ir);
        traverse(visited, max_id, jmp->target);
        break;
    }
    case IR_COND: {
        struct ir_cond *cond = ir->ir;
        mark_visited(visited, ir);
        traverse(visited, max_id, cond->target);
        traverse(visited, max_id, ir->next_else);
        break;
    }
    case IR_RET:
    case IR_RET_VOID: {
        mark_visited(visited, ir);
        if (ir->next)
            traverse(visited, max_id, ir->next);
        break;
    }
    default:
        break;
    }
}

static void remove_node(struct ir_node **ir, struct ir_node **head)
{
    /// Note: conditional statements is never removed, so
    ///       *next_else and *prev_else are unused.
    if ((*ir)->next) {
        (*ir)->next->prev = (*ir)->prev;
    }

    if ((*ir)->prev) {
        (*ir)->prev->next = (*ir)->next;
    } else {
        (*ir) = (*ir)->next;
        (*head) = (*ir);
    }
}

static void cut(bool *visited, int32_t siz, struct ir_node *ir)
{
    struct ir_node *it = ir;

    while (it) {
        if (ir->instr_idx <= siz && !visited[it->instr_idx]) {
            printf("Dead instruction at %d\n", it->instr_idx);
            remove_node(&it, &ir);
        }
        it = it->next;
    }
}

/// Traverse CFG and remove all unvisited nodes.
void ir_opt_unreachable_code(struct ir_func_decl *decl)
{
    bool visited[8192] = {0};
    /// How much nodes potentially were visited.
    int32_t max_id = 0;

    traverse(visited, &max_id, decl->body);
    cut(visited, max_id, decl->body);
}