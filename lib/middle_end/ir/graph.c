/* graph.c - Functions to build graph from IR.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "middle_end/ir/graph.h"
#include "middle_end/ir/dump.h"
#include "middle_end/ir/ir.h"
#include "middle_end/ir/ir_ops.h"
#include "util/compiler.h"
#include <assert.h>

static void control_flow_successors(
    struct ir_node *ir,
    struct ir_node **out_main,
    struct ir_node **out_alt
) {
    switch (ir->type) {
    case IR_COND: {
        struct ir_cond *cond = ir->ir;
        *out_main = cond->target;
        *out_alt = ir->next_else;
        break;
    }
    case IR_JUMP: {
        struct ir_jump *jmp = ir->ir;
        *out_main = jmp->target;
        *out_alt = NULL;
        break;
    }
    default: {
        *out_main = ir->next;
        *out_alt = NULL;
        break;
    }
    }
}

__weak_really_inline static void set_idom(
    struct ir_node  *node,
    struct ir_node  *idom,
    struct ir_node **worklist,
    uint64_t        *siz
) {
    if (node->idom == NULL) {
        node->idom = idom;
        if (idom->idom_back[0] == NULL)
            idom->idom_back[0] = node;
        else
            idom->idom_back[1] = node;
        worklist[(*siz)++] = node;
    }
}

/* https://www.cs.utexas.edu/users/misra/Lengauer+Tarjan.pdf
   https://www.cs.princeton.edu/courses/archive/fall03/cs528/handouts/a%20fast%20algorithm%20for%20finding.pdf */
static void dom_tree(struct ir_func_decl *decl)
{
    struct ir_node *root           = decl->body;
    struct ir_node *worklist[2048] = {0};
    uint64_t        siz            =  0;

    root->idom = root;

    worklist[siz++] = root;

    while (siz > 0) {
        struct ir_node *cur = worklist[--siz];

        struct ir_node *succs[2] = {0};
        control_flow_successors(cur, &succs[0], &succs[1]);

        if (succs[0])
            set_idom(succs[0], cur, worklist, &siz);

        if (succs[1])
            set_idom(succs[1], cur, worklist, &siz);
    }
}

/* This function constructs dominance frontier for given IR node.

   https://c9x.me/compile/bib/ssa.pdf

   for each X in a bottom-up traversal of the dominator tree do
     for each Y : Succ(X) do
       if idom(Y) != X
         then DF(X) <- DF(X) U {Y}   // local
     end
     for each Z : Children(X) do
       for each Y E DF(Z) do
         if idom(Y) != X
           then DF(X) <- DF(X) U {Y}   // up
       end
     end
   end */
static void dominance_frontier(struct ir_node *x)
{
    if (x->idom_back[0])
        dominance_frontier(x->idom_back[0]);

    if (x->idom_back[1])
        dominance_frontier(x->idom_back[1]);

    struct ir_node *succs[2] = {0};

    control_flow_successors(x, &succs[0], &succs[1]);

    for (uint64_t i = 0; i < 2; ++i) {
        struct ir_node *y = succs[i];
        if (y && y->idom != x)
            x->df[x->df_siz++] = y;
    }

    for (uint64_t i = 0; i < 2; ++i) {
        struct ir_node *z = x->idom_back[i];
        if (z) {
            for (uint64_t j = 0; j < z->df_siz; ++j) {
                struct ir_node *y = z->df[j];
                if (y && y->idom != x)
                    x->df[x->df_siz++] = y;
            }
        }
    }
}

void ir_compute_dom_tree(struct ir_node *ir)
{
    struct ir_node *it = ir;
    while (it) {
        dom_tree(it->ir);
        it = it->next;
    }
}

void ir_compute_dom_frontier(struct ir_node *decls)
{
    struct ir_node *it = decls;
    while (it) {
        struct ir_func_decl *decl = it->ir;
        dominance_frontier(decl->body);
        it = it->next;
    }
}

bool ir_dominated_by(struct ir_node *node, struct ir_node *dom)
{
    if (node == dom) return 1;

    while (node) {
        node = node->idom;
        if (node == dom) return 1;
    }
    return 0;
}

bool ir_dominates(struct ir_node *dom, struct ir_node *node)
{
    /// Note:
    /// It is possible to call there ir_is_dominated_by(),
    /// but this function is never inlineable in this context.
    if (dom == node) return 1;

    while (node) {
        node = node->idom;
        if (node == dom) return 1;
    }
    return 0;
}

/*
Steck' ein Messer in mein Bein und es kommt Blut raus
Doch die Schmerzen gehen vorbei
Meine Schwestern schreiben: „Bro, du siehst nicht gut aus“
Kann schon sein, weil ich bin high
*/