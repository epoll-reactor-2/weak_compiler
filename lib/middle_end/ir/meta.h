/* meta.h - Extra information about IR nodes.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_MIDDLE_END_META_H
#define WEAK_COMPILER_MIDDLE_END_META_H

#include "front_end/lex/data_type.h"
#include <stdint.h>
#include <stdbool.h>

struct type {
    enum data_type dt;
    bool           ptr;
    uint64_t       arity[16];
    uint64_t       arity_size;
    uint64_t       bytes;
};

enum { META_VALUE_UNKNOWN = UINT64_MAX };

struct meta {
    enum {
        IR_META_UNKNOWN = 0,
        IR_META_TYPE    = 1,
        IR_META_SYM     = 2,
        IR_META_FUN     = 4
    } kind;

    union {
        /** Variable information. */
        struct {
            bool    loop;
            bool    noalias;
            int32_t loop_idx;
        } sym;

        /** Function information. */
        struct {
            bool is_const;
        } fun;

        /** Type information. */
        struct type type;
    };

    /** Depth of current block. Needed to handle
        nested code blocks inside '{' and '}' in optimizations. */
    uint64_t nesting;

    /** Most outer loop index. Needed to know when
        to stop optimizing algorithms in case when
        loops are placed close. Without it
        we could incorrectly think, that shown below
        3 while's is the same loop because of same
        loop depth.

        while (a) { ... } /< Loop depth = 1
        <<< separator >>>

        while (b) { ... } /< Loop depth = 1
        <<< separator >>>

        while (c) { ... } /< Loop depth = 1
        <<< separator >>> */
    uint64_t global_loop_idx;

    /** On which condition instruction depends.
        Used in data-flow analysis. This points
        to most outer condition. It means,
        each statement in inner loop depends on
        most outer loop condition. Thus, outer loop
        "dominates" its whole body.

        \note Condition can be placed as above (for, while)
              so below (do-while). It means, we should walk
              upwards or downwards marking IR nodes as needed.

        int a = 1;
        int b = 2;
        int c = 3;

        if (c) {
            ++b;
        } else {
            ++a;
        }

        - b depends on b, c
        - a depends on a, c */
    uint64_t dominant_condition_idx;
};

#endif // WEAK_COMPILER_MIDDLE_END_META_H
