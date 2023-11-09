/* eval.c - Weak language IR interpreter.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "back_end/eval.h"
#include "middle_end/ir/dump.h"
#include "middle_end/ir/ir.h"
#include "util/crc32.h"
#include "util/hashmap.h"
#include "util/unreachable.h"
#include "util/vector.h"
#include <assert.h>
#include <string.h>



/* ==========================
   Stack routines.
   ========================== */

#define STACK_SIZE_BYTES 100000

/* TODO: Structure like `eval_result` with
         - immediate value type
           ` int
           ` char
           ` other primitive
           ` pointer
           ` static string
           ` structure
         - value itself. Maybe, union.

         Thus we will always know that we write and read from
         the stack. If we want to get value, it is enough to
         get pointer to stack. Not copying everything. */

/* NOTE: Such stack usage has sense only with reordered alloca
         instructions, where they are all collected at the beginning of
         the function. Thus we not require to injure our stack during
         loop execution. Each variable allocated once and set up
         multiple times.

         Moreover, language semantics forbid to have uninitialized
         values.

         Stack contains `struct eval_result`. */
static char     stack[STACK_SIZE_BYTES];
/* Index: sym_idx
   Value: sp */
static uint64_t stack_map[STACK_SIZE_BYTES];
/* Global stack pointer. Named as assembly register. */
static uint64_t sp;

/* Performance issue: each type (even if 1 byte)
   occurs 16 bytes in memory. */
struct eval_result {
    enum data_type dt;

    union {
        bool    __bool;
        char    __char;
        int32_t __int;
        float   __float;
        struct {
            uint64_t  siz;
            char     *value;
        } __string;
        struct {
            /* TODO. */
        } __struct;
    };
};



static void reset()
{
    memset(stack_map, 0, sizeof (stack_map));
    memset(stack, 0, sizeof (stack));
    sp = 0;
}

/* Notice: There is no `pop` function, since popping
           is implemented by storing stack pointer before
           call and restoring it after call. */
static inline void push(uint64_t sym_idx)
{
    uint64_t siz = sizeof (struct eval_result);

    stack_map[sym_idx] = sp;
    sp += siz;
}

static inline void set(uint64_t sym_idx, struct eval_result *er)
{
    uint64_t sp_ptr = stack_map[sym_idx];

    if (er->dt == D_T_UNKNOWN)
        weak_unreachable("D_T_UNKNOWN");

    memcpy(&stack[sp_ptr], er, sizeof (struct eval_result));
}

static inline struct eval_result *get(uint64_t sym_idx)
{
    uint64_t sp_ptr = stack_map[sym_idx];

    struct eval_result *er = (struct eval_result *) &stack[sp_ptr];

    if (er->dt == D_T_UNKNOWN)
        weak_unreachable("D_T_UNKNOWN");

    return er;
}



/* ==========================
   Instructions routines.
   ========================== */

static void call_eval(struct ir_func_call *call);
static void instr_eval(struct ir_node *ir);

static struct ir_node     *instr_ptr;
static struct eval_result  last;



static void eval_alloca(struct ir_alloca *alloca)
{
    push(alloca->idx);
}



static void eval_imm(struct ir_imm *imm)
{
    struct eval_result er = {0};
    switch (imm->type) {
    case IMM_BOOL:  er.dt = D_T_BOOL;  er.__bool  = imm->imm.__bool;  break;
    case IMM_CHAR:  er.dt = D_T_CHAR;  er.__char  = imm->imm.__char;  break;
    case IMM_FLOAT: er.dt = D_T_FLOAT; er.__float = imm->imm.__float; break;
    case IMM_INT:   er.dt = D_T_INT;   er.__int   = imm->imm.__int;   break;
    default:
        weak_unreachable("Should not reach there.");
    }
    memcpy(&last, &er, sizeof (struct eval_result));
}

static void eval_sym(struct ir_sym *sym)
{
    struct eval_result *er = get(sym->idx);
    memcpy(&last, er, sizeof (struct eval_result));
}



static void eval_bools(enum token_type op, bool l, bool r)
{
    last.dt = D_T_BOOL;

    switch (op) {
    case TOK_BIT_AND: last.__bool = l & r; break;
    case TOK_BIT_OR:  last.__bool = l | r; break;
    case TOK_XOR:     last.__bool = l ^ r; break;
    default:
        weak_unreachable("Unknown token type `%s`.", tok_to_string(op));
    }
}

static void eval_floats(enum token_type op, float l, float r)
{
    last.dt = D_T_FLOAT;

    switch (op) {
    case TOK_EQ:      last.__float = l == r; break;
    case TOK_NEQ:     last.__float = l != r; break;
    case TOK_GT:      last.__float = l  > r; break;
    case TOK_LT:      last.__float = l  < r; break;
    case TOK_GE:      last.__float = l >= r; break;
    case TOK_LE:      last.__float = l <= r; break;
    case TOK_PLUS:    last.__float = l  + r; break;
    case TOK_MINUS:   last.__float = l  - r; break;
    case TOK_STAR:    last.__float = l  * r; break;
    case TOK_SLASH:   last.__float = l  / r; break;
    default:
        weak_unreachable("Unknown token type `%s`.", tok_to_string(op));
    }
}

static void eval_ints(
    enum token_type  op,
    int32_t          l,
    int32_t          r
) {
    last.dt = D_T_INT;

    switch (op) {
    case TOK_AND:     last.__int = l && r; break;
    case TOK_OR:      last.__int = l || r; break;
    case TOK_XOR:     last.__int = l  ^ r; break;
    case TOK_BIT_AND: last.__int = l  & r; break;
    case TOK_BIT_OR:  last.__int = l  | r; break;
    case TOK_EQ:      last.__int = l == r; break;
    case TOK_NEQ:     last.__int = l != r; break;
    case TOK_GT:      last.__int = l  > r; break;
    case TOK_LT:      last.__int = l  < r; break;
    case TOK_GE:      last.__int = l >= r; break;
    case TOK_LE:      last.__int = l <= r; break;
    case TOK_SHL:     last.__int = l << r; break;
    case TOK_SHR:     last.__int = l >> r; break;
    case TOK_PLUS:    last.__int = l  + r; break;
    case TOK_MINUS:   last.__int = l  - r; break;
    case TOK_STAR:    last.__int = l  * r; break;
    case TOK_SLASH:   last.__int = l  / r; break;
    case TOK_MOD:     last.__int = l  % r; break;
    default:
        weak_unreachable("Unknown token type `%s`.", tok_to_string(op));
    }
}

static void eval_chars(
    enum token_type  op,
    int32_t          l,
    int32_t          r
) {
    last.dt = D_T_CHAR;

    switch (op) {
    case TOK_AND:     last.__char = l && r; break;
    case TOK_OR:      last.__char = l || r; break;
    case TOK_XOR:     last.__char = l  ^ r; break;
    case TOK_BIT_AND: last.__char = l  & r; break;
    case TOK_BIT_OR:  last.__char = l  | r; break;
    case TOK_EQ:      last.__char = l == r; break;
    case TOK_NEQ:     last.__char = l != r; break;
    case TOK_GT:      last.__char = l  > r; break;
    case TOK_LT:      last.__char = l  < r; break;
    case TOK_GE:      last.__char = l >= r; break;
    case TOK_LE:      last.__char = l <= r; break;
    case TOK_SHL:     last.__char = l << r; break;
    case TOK_SHR:     last.__char = l >> r; break;
    case TOK_PLUS:    last.__char = l  + r; break;
    case TOK_MINUS:   last.__char = l  - r; break;
    case TOK_STAR:    last.__char = l  * r; break;
    case TOK_SLASH:   last.__char = l  / r; break;
    case TOK_MOD:     last.__char = l  % r; break;
    default:
        weak_unreachable("Unknown token type `%s`.", tok_to_string(op));
    }
}


static void compute(enum token_type op, struct eval_result *l, struct eval_result *r)
{
    if (l->dt != r->dt)
        weak_unreachable("dt(L) = %s, dt(R) = %s", data_type_to_string(l->dt), data_type_to_string(r->dt));

    switch (l->dt) {
    case D_T_BOOL:  eval_bools (op, l->__bool , r->__bool ); break;
    case D_T_CHAR:  eval_chars (op, l->__char , r->__char ); break;
    case D_T_INT:   eval_ints  (op, l->__int  , r->__int  ); break;
    case D_T_FLOAT: eval_floats(op, l->__float, r->__float); break;
    default:
        weak_unreachable("Unknown immediate type (numeric: %d).", l->dt);
    }
}

static void eval_bin(struct ir_bin *bin)
{
    instr_eval(bin->lhs);
    struct eval_result l = last;

    instr_eval(bin->rhs);
    struct eval_result r = last;

    compute(bin->op, &l, &r);
}



static void eval_store_imm(struct ir_store *store)
{
    assert(store->idx->type == IR_SYM && "TODO: Implement arrays");

    struct ir_imm *imm = store->body->ir;
    struct ir_sym *sym = store->idx->ir;

    switch (imm->type) {
    case IMM_BOOL:  last.dt = D_T_BOOL;  last.__bool  = imm->imm.__bool;  break;
    case IMM_CHAR:  last.dt = D_T_CHAR;  last.__char  = imm->imm.__char;  break;
    case IMM_FLOAT: last.dt = D_T_FLOAT; last.__float = imm->imm.__float; break;
    case IMM_INT:   last.dt = D_T_INT;   last.__int   = imm->imm.__int;   break;
    default:
        weak_unreachable("Should not reach there");
    }

    set(sym->idx, &last);
}

static void eval_store_sym(struct ir_store *store)
{
    /* Copy from one stack location to another. */
    assert(store->idx->type == IR_SYM && "TODO: Implement arrays");

    struct ir_sym *from = store->body->ir;
    struct ir_sym *to   = store->idx->ir;

    struct eval_result *er = get(from->idx);
    set(to->idx, er);
}

static void eval_store_bin(struct ir_store *store)
{
    instr_eval(store->body);
    assert(store->idx->type == IR_SYM && "TODO: Implement arrays");

    struct ir_sym *sym = store->idx->ir;
    set(sym->idx, &last);
}

static void eval_store_call(struct ir_store *store)
{
    instr_eval(store->body);
    assert(store->idx->type == IR_SYM && "TODO: Implement arrays");

    struct ir_sym *sym = store->idx->ir;
    set(sym->idx, &last);
}

static void eval_store(struct ir_store *store)
{
    switch (store->body->type) {
    case IR_IMM: {
        eval_store_imm(store);
        break;
    }
    case IR_SYM: {
        eval_store_sym(store);
        break;
    }
    case IR_BIN: {
        eval_store_bin(store);
        break;
    }
    case IR_FUNC_CALL: {
        eval_store_call(store);
        break;
    }
    default:
        break;
    }

}


static void eval_jmp(struct ir_node *jmp)
{
    instr_ptr = vector_at(jmp->cfg.succs, 0);
}

static void eval_cond(struct ir_node *__cond)
{
    struct ir_cond *cond = __cond->ir;
    instr_eval(cond->cond);

    bool should_jump;
    switch (last.dt) {
    case D_T_BOOL:  should_jump = last.__bool; break;
    case D_T_CHAR:  should_jump = last.__char != '\0'; break;
    case D_T_FLOAT: should_jump = last.__float != 0.0; break;
    case D_T_INT:   should_jump = last.__int != 0; break;
    default:
        weak_unreachable("Unknown immediate type (numeric: %d).", last.dt);
    }

    if (should_jump)
        instr_ptr = cond->target; /* True branch. */
    else
        instr_ptr = __cond->next; /* False branch. */
}

static void eval_ret(struct ir_ret *ret)
{
    if (ret->body)
        instr_eval(ret->body);

    instr_ptr = NULL;
}



static void instr_eval(struct ir_node *ir)
{
    switch (ir->type) {
    case IR_ALLOCA:
        eval_alloca(ir->ir);
        break;
    case IR_IMM:
        eval_imm(ir->ir);
        break;
    case IR_SYM:
        eval_sym(ir->ir);
        break;
    case IR_JUMP:
        eval_jmp(ir);
        break;
    case IR_MEMBER:
    case IR_TYPE_DECL:
    case IR_FUNC_DECL:
        break;
    case IR_FUNC_CALL:
        call_eval(ir->ir);
        break;
    case IR_STORE:
        eval_store(ir->ir);
        break;
    case IR_BIN:
        eval_bin(ir->ir);
        break;
    case IR_RET:
    case IR_RET_VOID:
        eval_ret(ir->ir);
        break;
    case IR_COND:
        eval_cond(ir);
        break;
    default:
        weak_unreachable("Unknown IR type (numeric: %d).", ir->type);
    }
}



/* ==========================
   Call stack.
   ========================== */

/* Important notice about drawing stacks in LaTeX
   https://tex.stackexchange.com/questions/235000/drawing-an-activation-stack-in-latex */

static void printf_n(uint32_t count, char c)
{
    for (uint32_t i = 0; i < count; ++i) {
        putc(i % 2 != 0 ? c : '|', stdout);
    }
}

struct call_stack_entry {
    const char *name;
    uint64_t    sp;
};

typedef vector_t(struct call_stack_entry) call_stack_t;

static uint64_t call_depth;

/* TODO: Builtin function that prints stacktrace at
         the moment.

         strace();
         ` prints
         `
         call `main` (+0)
           call `fact` (+24)
             call `fact` (+144)
               call `fact` (+264)
                 call `fact` (+384)
                  call `fact` (+504)
*/
static void call_stack_head(const char *fname)
{
    printf_n(call_depth, ' ');
    printf("call `%s` (+%ld)\n", fname, sp);
    call_depth += 2;
}

static void call_stack_tail()
{
    call_depth -= 2;
}



/* ==========================
   Functions routines.
   ========================== */

static void reset_hashmap(hashmap_t *map, uint64_t siz)
{
    if (map->buckets) {
        hashmap_destroy(map);
    }
    hashmap_init(map, siz);
}

static hashmap_t funs;

static void fun_list_init(struct ir_node *ir)
{
    while (ir) {
        struct ir_func_decl *fun = ir->ir;
        hashmap_put(&funs, crc32_string(fun->name), (uint64_t) fun);
        ir = ir->next;
    }
}

static struct ir_func_decl *fun_lookup(const char *name)
{
    uint64_t hash = crc32_string(name);

    bool ok = 0;
    uint64_t got = hashmap_get(&funs, hash, &ok);
    if (!ok)
        weak_unreachable("Function lookup failed for `%s`, CRC32: %ld", name, hash);

    return (struct ir_func_decl *) got;
}

static void fun_eval(struct ir_func_decl *decl)
{
    struct ir_node *it = decl->body;

    instr_ptr = it;

    while (instr_ptr) {
        struct ir_node *prev_ptr = instr_ptr;
        instr_eval(instr_ptr);

        /* Conditional and jump statements set up their
           successor instructions manually. Elsewise,
           we just peek the next one. */
        switch (prev_ptr->type) {
        case IR_COND:
        case IR_JUMP:
            break;
        default:
            if (instr_ptr)
                instr_ptr = vector_at(instr_ptr->cfg.succs, 0);
        }
    }
}



static void call_eval(struct ir_func_call *fcall)
{
    call_stack_head(fcall->name);

    uint64_t sym = 0;
    uint64_t save_sp = sp;
    struct ir_node *save_instr_ptr = instr_ptr;

    uint64_t stack_map_copy[STACK_SIZE_BYTES];

    memcpy(stack_map_copy, stack_map, STACK_SIZE_BYTES);

    struct ir_node *arg = fcall->args;
    while (arg) {
        /* Evaluate in current stack frame. */
        instr_eval(arg);
        /* Push to the callee stack frame. */
        push(sym);
        set(sym++, &last);
        arg = arg->next;
    }

    struct ir_func_decl *fun = fun_lookup(fcall->name);

    fun_eval(fun);
    call_stack_tail();

    sp = save_sp;
    instr_ptr = save_instr_ptr;
    memcpy(stack_map, stack_map_copy, STACK_SIZE_BYTES);
}



/* ==========================
   Driver code.
   ========================== */

int32_t eval(struct ir_node *ir)
{
    reset();
    reset_hashmap(&funs, 512);

    fun_list_init(ir);

    struct ir_func_call main = {
        .name = "main"
    };
    call_eval(&main);

    /* Required to be int. */
    if (last.dt != D_T_INT)
        weak_unreachable("main() return only ints.");

    return last.__int;
}