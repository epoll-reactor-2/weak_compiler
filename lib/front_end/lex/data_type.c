/* data_type.c - Definition of data types.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/lex/data_type.h"
#include "util/unreachable.h"

/* extern */ int data_type_size[D_T_TOTAL] = {
    [D_T_FUNC]       = 0,
    [D_T_STRUCT]     = 0,
    [D_T_VOID]       = 0,
    [D_T_CHAR]       = 1,
    [D_T_SHORT]      = 2,
    [D_T_INT]        = 4,
    [D_T_LONG]       = 8,
    [D_T_FLOAT]      = 4,
    [D_T_DOUBLE]     = 8,
    [D_T_SIGNED]     = 4,
    [D_T_UNSIGNED]   = 4,
    [D_T_BOOL]       = 1,
    [D_T_COMPLEX]    = -1, /* Complex float, double. */
};