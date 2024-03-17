/* data_type.h - Definition of data types.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_FRONTEND_LEX_DATA_TYPE_H
#define FCC_FRONTEND_LEX_DATA_TYPE_H

#include "util/compiler.h"

#define __take_enum(x, y) x,
#define __take_string(x, y) y,

#define map(take) \
    take(D_T_UNKNOWN, "<unknown>") \
    take(D_T_FUNC, "function") \
    take(D_T_STRUCT, "struct") \
    take(D_T_VOID, "void") \
    take(D_T_CHAR, "char") \
    take(D_T_SHORT, "short") \
    take(D_T_INT, "int") \
    take(D_T_LONG, "long") \
    take(D_T_FLOAT, "float") \
    take(D_T_DOUBLE, "double") \
    take(D_T_SIGNED, "signed") \
    take(D_T_UNSIGNED, "unsigned") \
    take(D_T_BOOL, "bool") \
    take(D_T_COMPLEX, "complex") \
    take(D_T_TOTAL, "<total>")

enum data_type {
    map(__take_enum)
};

really_inline static const char *data_type_to_string(int t)
{
    static const char *names[] = { map(__take_string) };
    return names[t];
}

extern int data_type_size[D_T_TOTAL];

#undef __take_string
#undef __take_enum
#undef map

#endif // FCC_FRONTEND_LEX_DATA_TYPE_H