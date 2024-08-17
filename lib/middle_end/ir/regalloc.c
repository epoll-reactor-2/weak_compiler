/* regalloc.c - Register allocator.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "middle_end/ir/regalloc.h"
#include "middle_end/ir/ir.h"
#include <assert.h>
#include <stdint.h>

/**********************************************
 **       Graph-coloring allocator           **
 **********************************************/

 #define REG_ALLOC_MAX_VARS 512
 #define REG_ALLOC_MAX_REGS   8   /**< TODO: Hardware-specific? */

struct interference_graph {
    int graph[REG_ALLOC_MAX_VARS][REG_ALLOC_MAX_VARS];
    int degree[REG_ALLOC_MAX_VARS];
};

struct live_range {
    int start;
    int end;
};

struct live_range_info {
    struct live_range ranges[REG_ALLOC_MAX_VARS];
    int               count;
};

struct reg_allocator {
    int  color[REG_ALLOC_MAX_VARS];
    bool spill[REG_ALLOC_MAX_VARS];
};

static void reg_alloc_add_edge(struct interference_graph *g, int u, int v)
{
    if (!g->graph[u][v]) {
        g->graph[u][v] = 1;
        g->graph[v][u] = 1;
        g->degree[u]++;
        g->degree[v]++;
    }
}

static void reg_alloc_build_graph(struct interference_graph *g, struct live_range_info *info)
{
    for (uint64_t i = 0; i < info->count; ++i) {
        for (uint64_t j = i + 1; j < info->count; ++j) {
            if (info->ranges[i].start <= info->ranges[j].end &&
                info->ranges[j].start <= info->ranges[i].end)
                reg_alloc_add_edge(g, i, j);
        }
    }
}

static void reg_alloc(struct interference_graph *g, struct reg_allocator *allocator)
{
    for (uint64_t i = 0; i < REG_ALLOC_MAX_VARS; ++i) {
        allocator->color[i] = -1;
        allocator->spill[i] =  0;
    }

    bool available[REG_ALLOC_MAX_REGS] = {0};

    for (uint64_t i = 0; i < REG_ALLOC_MAX_VARS; ++i) {
        for (uint64_t j = 0; j < REG_ALLOC_MAX_REGS; ++j) {
            available[j] = 1;
        }

        for (uint64_t j = 0; j < REG_ALLOC_MAX_VARS; ++j) {
            if (g->graph[i][j] && allocator->color[j] != -1)
                available[allocator->color[j]] = 0;
        }

        int reg = -1;
        for (uint64_t j = 0; j < REG_ALLOC_MAX_REGS; ++j) {
            if (available[j]) {
                reg = j;
                break;
            }
        }

        if (reg != -1)
            allocator->color[i] = reg;
        else
            allocator->spill[i] = 1;
    }
}

/**********************************************
 **        Allocator initialization          **
 **********************************************/

static void reg_alloc_store_op_idx(struct ir_node *ir, int *arg1, int *arg2)
{
    *arg1 = -1;
    *arg2 = -1;

    switch (ir->type) {
    case IR_SYM: {
        struct ir_sym *sym = ir->ir;
        *arg1 = sym->idx;
        break;
    }
    case IR_BIN: {
        struct ir_bin *bin = ir->ir;

        if (bin->lhs->type == IR_SYM) {
            struct ir_sym *sym = bin->lhs->ir;
            *arg1 = sym->idx;
        }

        if (bin->rhs->type == IR_SYM) {
            struct ir_sym *sym = bin->rhs->ir;
            *arg2 = sym->idx;
        }

        break;
    }
    default:
        break;
    }
}

static void reg_alloc_live_ranges(struct live_range_info *info, struct ir_node *ir)
{
    for (uint64_t i = 0; i < REG_ALLOC_MAX_VARS; ++i) {
        info->ranges[i].start = -1;
        info->ranges[i].end   = -1;
    }

    while (ir) {
        switch (ir->type) {
        case IR_STORE: {
            struct ir_store *store = ir->ir;
            assert(store->idx->type == IR_SYM);

            struct ir_sym *sym = store->idx->ir;
            int res = sym->idx;
            /* NOTE: Maybe not this. */
            int i = ir->instr_idx;

            /* Result. */
            if (info->ranges[res].start == -1) {
                info->ranges[res].start = i;
            }
            info->ranges[res].end = i;

            int arg1 = 0;
            int arg2 = 0;

            reg_alloc_store_op_idx(store->body, &arg1, &arg2);

            /* Left argument. */
            if (arg1 != -1) {
                if (info->ranges[arg1].start == -1) {
                    info->ranges[arg1].start = i;
                }
                info->ranges[arg1].end = i;
            }

            /* Right argument. */
            if (arg2 != -1) {
                if (info->ranges[arg2].start == -1) {
                    info->ranges[arg2].start = i;
                }
                info->ranges[arg2].end = i;
            }

            break;
        }
        default:
            break;
        }

        ir = ir->next;
    }

    info->count = REG_ALLOC_MAX_VARS;
}

/**********************************************
 **       Register -> IR assignment          **
 **********************************************/

static void reg_alloc_assign_regs(struct ir_node *it, struct reg_allocator *allocator)
{
    while (it) {
        if (it->type == IR_STORE) {
            struct ir_store *store = it->ir;
            struct ir_sym   *sym   = store->idx->ir;

            if (!allocator->spill[sym->idx])
                it->claimed_reg = allocator->color[sym->idx];
        }

        it = it->next;
    }
}

/**********************************************
 **                Traversal                 **
 **********************************************/

static void reg_alloc_fn(struct ir_fn_decl *ir)
{
    /* TODO: function args. */
    struct interference_graph graph           = {0};
    struct live_range_info    live_range_info = {0};
    struct reg_allocator      allocator       = {0};

    reg_alloc_live_ranges(&live_range_info, ir->body);
    reg_alloc_build_graph(&graph, &live_range_info);
    reg_alloc(&graph, &allocator);
    reg_alloc_assign_regs(ir->body, &allocator);
}

void ir_reg_alloc(struct ir_node *functions)
{
    struct ir_node *it = functions;
    while (it) {
        struct ir_fn_decl *decl = it->ir;

        reg_alloc_fn(decl);

        it = it->next;
    }
}