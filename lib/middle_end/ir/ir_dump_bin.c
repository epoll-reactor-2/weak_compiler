/* ir_dump_bin.c - Generate binary files with weak IR.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "middle_end/ir/gen.h"
#include "middle_end/ir/ir.h"
#include "middle_end/ir/ir_dump.h"
#include "middle_end/ir/ir_dump_bin.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

#if 0
#define DUMP(x, bytes) \
    { for (uint64_t i = 0; i < bytes; ++i) { \
        printf("%X ", ((char *)(x))[i]); \
      } puts(""); }
#else
#define DUMP(...)
#endif

#define ir_fwrite(x) \
    { fwrite (&(x), sizeof ((x)), 1, mem); \
      DUMP (&(x), sizeof ((x))); \
    }

#define ir_fwrite_ptr(x) \
    { fwrite ((x), sizeof (*(x)), 1, mem); \
      DUMP ((x), sizeof (*(x))); \
    }

#define ir_fwrite_bytes(x, n) \
    { fwrite ((x), (n), 1, mem); \
      DUMP (&(x), sizeof ((x))); \
    }

#define ir_fread(x) \
    { int rc = fread (&(x), sizeof ((x)), 1, mem); \
      if (rc != 1) \
        weak_fatal_errno("fread(%d)", rc); \
      DUMP (&(x), sizeof ((x))); }

#define ir_fread_ptr(x) \
    { int rc = fread ((x), sizeof (*(x)), 1, mem); \
      if (rc != 1) \
        weak_fatal_errno("fread(%d)", rc); \
      DUMP ((x), sizeof (*(x))); }

#define ir_fread_bytes(x, n) \
    { int rc = fread ((x), (n), 1, mem); \
      if (rc != 1) \
        weak_fatal_errno("fread(%d)", rc); \
      DUMP ((x), (n)); }

/**********************************************
 **            Dump functions                **
 **********************************************/
void dump_node(FILE *mem, struct ir_node *ir);
struct ir_node *read_node(FILE *mem);

/**********************************************
 **                 Alloca                   **
 **********************************************/
void dump_alloca(FILE *mem, struct ir_node *ir)
{
    struct ir_alloca *alloca = ir->ir;
    ir_fwrite_ptr(alloca);
}

void read_alloca(unused FILE *mem, struct ir_node *ir)
{
    struct ir_alloca *alloca = weak_new(struct ir_alloca);
    ir->ir = alloca;
    ir_fread_ptr(alloca);
}

/**********************************************
 **               Alloca array               **
 **********************************************/
void dump_alloca_array(FILE *mem, struct ir_node *ir)
{
    struct ir_alloca_array *alloca = ir->ir;
    ir_fwrite_ptr(alloca);
}

void read_alloca_array(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_alloca_array *alloca = weak_new(struct ir_alloca_array);
    ir->ir = alloca;
    ir_fread_ptr(alloca);
}

/**********************************************
 **               Immediate                  **
 **********************************************/
void dump_imm(FILE *mem, struct ir_node *ir)
{
    struct ir_imm *imm = ir->ir;
    ir_fwrite_ptr(imm);
}

void read_imm(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_imm *imm = weak_new(struct ir_imm);
    ir->ir = imm;

    ir_fread_ptr(imm);
}

/**********************************************
 **                 String                   **
 **********************************************/
void dump_string(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_string *s = ir->ir;
    ir_fwrite(s->len);
    ir_fwrite_bytes(s->imm, s->len);
}

void read_string(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_string *s = weak_new(struct ir_string);
    ir->ir = s;

    ir_fread(s->len);
    s->imm = weak_calloc(1, s->len);
    ir_fread_bytes(s->imm, s->len);
}

/**********************************************
 **                 Symbol                   **
 **********************************************/
void dump_sym(FILE *mem, struct ir_node *ir)
{
    struct ir_sym *sym = ir->ir;
    ir_fwrite_ptr(sym);
}

void read_sym(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_sym *sym = weak_new(struct ir_sym);
    ir->ir = sym;

    ir_fread_ptr(sym);
}

/**********************************************
 **                 Store                    **
 **********************************************/
void dump_store(FILE *mem, struct ir_node *ir)
{
    struct ir_store *store = ir->ir;
    dump_node(mem, store->idx);
    dump_node(mem, store->body);
}

void read_store(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_store *store = weak_new(struct ir_store);
    ir->ir = store;

    store->idx  = read_node(mem);
    store->body = read_node(mem);
}

/**********************************************
 **                Binary                    **
 **********************************************/
void dump_bin(FILE *mem, struct ir_node *ir)
{
    struct ir_bin *bin = ir->ir;
    ir_fwrite(bin->op);
    dump_node(mem, bin->lhs);
    dump_node(mem, bin->rhs);
}

void read_bin(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_bin *bin = weak_new(struct ir_bin);
    ir->ir = bin;

    ir_fread(bin->op);
    bin->lhs = read_node(mem);
    bin->rhs = read_node(mem);
}

/**********************************************
 **                 Jump                     **
 **********************************************/
void dump_jump(FILE *mem, struct ir_node *ir)
{
    struct ir_jump *jump = ir->ir;
    ir_fwrite(jump->idx);
}

void read_jump(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_jump *jump = weak_new(struct ir_jump);
    ir->ir = jump;

    ir_fread(jump->idx);
}

/**********************************************
 **              Conditional                 **
 **********************************************/
void dump_cond(FILE *mem, struct ir_node *ir)
{
    struct ir_cond *cond = ir->ir;
    dump_node(mem, cond->cond);
    ir_fwrite(cond->goto_label);
}

void read_cond(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_cond *cond = weak_new(struct ir_cond);
    ir->ir = cond;

    cond->cond = read_node(mem);
    ir_fread(cond->goto_label);
}

/**********************************************
 **                Return                    **
 **********************************************/
void dump_ret(FILE *mem, struct ir_node *ir)
{
    struct ir_ret *ret = ir->ir;

    ir_fwrite(ret->is_void);
    if (!ret->is_void)
        dump_node(mem, ret->body);
}

void read_ret(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_ret *ret = weak_new(struct ir_ret);
    ir->ir = ret;

    ir_fread(ret->is_void);
    if (!ret->is_void)
        ret->body = read_node(mem);
}

/**********************************************
 **                Member                    **
 **********************************************/
void dump_member(unused FILE *mem, unused struct ir_node *ir)
{}

void read_member(unused FILE *mem, unused struct ir_node *ir)
{}

/**********************************************
 **              Type declarator             **
 **********************************************/
void dump_type_decl(unused FILE *mem, unused struct ir_node *ir)
{}

void read_type_decl(unused FILE *mem, unused struct ir_node *ir)
{}

/**********************************************
 **                  Call                    **
 **********************************************/
void dump_fn_call(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_fn_call *call = ir->ir;

    uint64_t len = strlen(call->name);
    ir_fwrite(len);
    ir_fwrite_bytes(call->name, len);

    uint64_t args = 0;
    struct ir_node *it = call->args;
    while (it) {
        ++args;
        it = it->next;
    }
    ir_fwrite(args);

    it = call->args;
    while (it) {
        dump_node(mem, it);
        it = it->next;
    }
}

void read_fn_call(unused FILE *mem, unused struct ir_node *ir)
{
    struct ir_fn_call *call = weak_new(struct ir_fn_call);
    ir->ir = call;

    uint64_t len = 0;
    ir_fread(len);
    call->name = weak_calloc(1, len);
    ir_fread_bytes(call->name, len);

    uint64_t args_num = 0;
    ir_fread(args_num);

    if (args_num == 0)
        return;

    ir_vector_t args = {0};

    for (uint64_t i = 0; i < args_num; ++i) {
        vector_push_back(args, read_node(mem));
    }

    call->args = args.data[0];

    vector_foreach(args, i) {
        if (i >= args.count - 1)
            break;
        vector_at(args, i)->next =
        vector_at(args, i + 1);
    }
}

/**********************************************
 **                  Phi                     **
 **********************************************/
void dump_phi(unused FILE *mem, unused struct ir_node *ir)
{}

void read_phi(unused FILE *mem, unused struct ir_node *ir)
{}

/**********************************************
 **      Function declarator (header)        **
 **********************************************/
void dump_fn_decl_header(FILE *mem, struct ir_fn_decl *decl)
{
    uint64_t len = strlen(decl->name);
    ir_fwrite(len);
    ir_fwrite_bytes(decl->name, len);
    ir_fwrite(decl->ret_type);
    ir_fwrite(decl->ptr_depth);
}

void read_fn_decl_header(FILE *mem, struct ir_fn_decl *decl)
{
    uint64_t len = 0;
    ir_fread(len);
    decl->name = weak_calloc(1, len);
    ir_fread_bytes(decl->name, len);
    ir_fread(decl->ret_type);
    ir_fread(decl->ptr_depth);
}

/**********************************************
 **       Function declarator (args)         **
 **********************************************/
void dump_fn_decl_args(FILE *mem, struct ir_fn_decl *decl)
{
    uint64_t num = 0;
    struct ir_node *it = decl->args;

    while (it) {
        ++num;
        it = it->next;
    }
    ir_fwrite(num);
    it = decl->args;
    while (it) {
        dump_alloca(mem, it);
        it = it->next;
    }
}

void read_fn_decl_args(FILE *mem, struct ir_fn_decl *decl)
{
    uint64_t num = 0;

    ir_fread(num);

    ir_vector_t args = {0};
    for (uint64_t i = 0; i < num; ++i) {
        struct ir_node *ir = weak_new(struct ir_node);
        read_alloca(mem, ir);
        vector_push_back(args, ir);
    }

    if (num == 0)
        return;

    decl->args = args.data[0];

    vector_foreach(args, i) {
        if (i >= args.count - 1)
            break;
        vector_at(args, i)->next =
        vector_at(args, i + 1);
    }
}

/**********************************************
 **      Function declarator (body)          **
 **********************************************/
void dump_fn_decl_body(FILE *mem, struct ir_fn_decl *decl)
{
    uint64_t num = 0;
    struct ir_node *it = decl->body;

    while (it) {
        ++num;
        it = it->next;
    }
    ir_fwrite(num);
    it = decl->body;
    while (it) {
        dump_node(mem, it);
        it = it->next;
    }
}

void read_fn_decl_body(FILE *mem, struct ir_fn_decl *decl)
{
    uint64_t num = 0;
    ir_vector_t stmts = {0};

    ir_fread(num);
    for (uint64_t i = 0; i < num; ++i)
        vector_push_back(stmts, read_node(mem));

    vector_foreach(stmts, i) {
        if (i >= stmts.count - 1)
            break;
        vector_at(stmts, i)->next =
        vector_at(stmts, i + 1);
    }

    decl->body = vector_at(stmts, 0);
    ir_cfg_build(decl);
}

/**********************************************
 **          Function declarator             **
 **********************************************/
void dump_fn_decl(FILE *mem, struct ir_node *ir)
{
    struct ir_fn_decl *decl = ir->ir;
    dump_fn_decl_header(mem, decl);
    dump_fn_decl_args(mem, decl);
    dump_fn_decl_body(mem, decl);
}

void read_fn_decl(FILE *mem, unused struct ir_node *ir)
{
    ir->ir = weak_new(struct ir_fn_decl);
    read_fn_decl_header(mem, ir->ir);
    read_fn_decl_args(mem, ir->ir);
    read_fn_decl_body(mem, ir->ir);
}

/**********************************************
 **                  Node                    **
 **********************************************/
void dump_node_meta(FILE *mem, struct ir_node *ir)
{
    ir_fwrite(ir->type);
    ir_fwrite(ir->instr_idx);
    ir_fwrite(ir->cfg_block_no);
    ir_fwrite(ir->meta);
}

void read_node_meta(FILE *mem, struct ir_node *ir)
{
    ir_fread(ir->type);
    ir_fread(ir->instr_idx);
    ir_fread(ir->cfg_block_no);
    ir_fread(ir->meta);
}

void dump_node(FILE *mem, struct ir_node *ir)
{
    /* printf("IR write type: %s\n", ir_type_to_string(ir->type)); */
    dump_node_meta(mem, ir);

    switch (ir->type) {
    case IR_ALLOCA:       dump_alloca(mem, ir); break;
    case IR_ALLOCA_ARRAY: dump_alloca_array(mem, ir); break;
    case IR_IMM:          dump_imm(mem, ir); break;
    case IR_STRING:       dump_string(mem, ir); break;
    case IR_SYM:          dump_sym(mem, ir); break;
    case IR_STORE:        dump_store(mem, ir); break;
    case IR_BIN:          dump_bin(mem, ir); break;
    case IR_JUMP:         dump_jump(mem, ir); break;
    case IR_COND:         dump_cond(mem, ir); break;
    case IR_RET:          dump_ret(mem, ir); break;
    case IR_MEMBER:       dump_member(mem, ir); break;
    case IR_TYPE_DECL:    dump_type_decl(mem, ir); break;
    case IR_FN_DECL:      dump_fn_decl(mem, ir); break;
    case IR_FN_CALL:      dump_fn_call(mem, ir); break;
    case IR_PHI:          dump_phi(mem, ir); break;
    default:
        weak_unreachable("Unknown IR type (numeric: %d).", ir->type);
    }
}

struct ir_node *read_node(FILE *mem)
{
    struct ir_node *ir = weak_new(struct ir_node);

    read_node_meta(mem, ir);
    /* printf("IR read type: %s\n", ir_type_to_string(ir->type)); */

    switch (ir->type) {
    case IR_ALLOCA:       read_alloca(mem, ir); break;
    case IR_ALLOCA_ARRAY: read_alloca_array(mem, ir); break;
    case IR_IMM:          read_imm(mem, ir); break;
    case IR_STRING:       read_string(mem, ir); break;
    case IR_SYM:          read_sym(mem, ir); break;
    case IR_STORE:        read_store(mem, ir); break;
    case IR_BIN:          read_bin(mem, ir); break;
    case IR_JUMP:         read_jump(mem, ir); break;
    case IR_COND:         read_cond(mem, ir); break;
    case IR_RET:          read_ret(mem, ir); break;
    case IR_MEMBER:       read_member(mem, ir); break;
    case IR_TYPE_DECL:    read_type_decl(mem, ir); break;
    case IR_FN_DECL:      read_fn_decl(mem, ir); break;
    case IR_FN_CALL:      read_fn_call(mem, ir); break;
    case IR_PHI:          read_phi(mem, ir); break;
    default:
        weak_unreachable("Unknown IR type (numeric: %d).", ir->type);
    }

    return ir;
}

/**********************************************
 **                  Unit                    **
 **********************************************/
void dump_unit(FILE *mem, struct ir_unit *ir)
{
    struct ir_node *it = ir->fn_decls;
    uint64_t num_fns = 0;
    while (it) {
        ++num_fns;
        it = it->next;
    }

    ir_fwrite(num_fns);

    it = ir->fn_decls;
    while (it) {
        dump_fn_decl(mem, it);
        it = it->next;
    }
}

struct ir_unit read_unit(FILE *mem)
{
    uint64_t num_fns = 0;
    struct ir_unit unit = {
        .fn_decls = weak_new(struct ir_node)
    };

    ir_fread(num_fns);

    /* printf("Total fns: %lu\n", num_fns); */

    for (uint64_t i = 0; i < num_fns; ++i)
        read_fn_decl(mem, unit.fn_decls);

    return unit;
}

/**********************************************
 **              Driver code                 **
 **********************************************/
void ir_dump_binary(struct ir_unit *unit, const char *filename)
{
    FILE *s = fopen(filename, "wb");
    dump_unit(s, unit);
    fflush(s);
    fclose(s);
}

struct ir_unit ir_read_binary(const char *filename)
{
    FILE *s = fopen(filename, "rb");

    struct ir_unit unit = read_unit(s);
    fflush(s);
    fclose(s);

    return unit;
}