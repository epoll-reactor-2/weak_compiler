/* ast_type.h - List of all AST types.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_FRONTEND_AST_AST_TYPE_H
#define FCC_FRONTEND_AST_AST_TYPE_H

#include "util/compiler.h"

#define __take_enum(x, y) x,
#define __take_string(x, y) y,

#define map(take) \
    /** Literals. */ \
    take(AST_CHAR, "AST_CHAR") \
    take(AST_INT, "AST_INT") \
    take(AST_FLOAT, "AST_FLOAT") \
    take(AST_STRING, "AST_STRING") \
    take(AST_BOOL, "AST_BOOL") \
    /** Variable reference. */ \
    take(AST_SYMBOL, "AST_SYMBOL") \
    /** Declarations. */ \
    take(AST_VAR_DECL, "AST_VAR_DECL") \
    take(AST_ARRAY_DECL, "AST_ARRAY_DECL") \
    take(AST_STRUCT_DECL, "AST_STRUCT_DECL") \
    /** Iteration statements. */ \
    take(AST_BREAK_STMT, "AST_BREAK_STMT") \
    take(AST_CONTINUE_STMT, "AST_CONTINUE_STMT") \
    /** Operators. */ \
    take(AST_BINARY, "AST_BINARY") \
    take(AST_PREFIX_UNARY, "AST_PREFIX_UNARY") \
    take(AST_POSTFIX_UNARY, "AST_POSTFIX_UNARY") \
    take(AST_ARRAY_ACCESS, "AST_ARRAY_ACCESS") \
    take(AST_MEMBER, "AST_MEMBER") \
    /** Selection statements. */ \
    take(AST_IF_STMT, "AST_IF_STMT") \
    /** Loops. */ \
    take(AST_FOR_STMT, "AST_FOR_STMT") \
    take(AST_FOR_RANGE_STMT, "AST_FOR_RANGE_STMT") \
    take(AST_WHILE_STMT, "AST_WHILE_STMT") \
    take(AST_DO_WHILE_STMT, "AST_DO_WHILE_STMT") \
    /** Jump statements. */ \
    take(AST_RETURN_STMT, "AST_RETURN_STMT") \
    /** Block statements. */ \
    take(AST_COMPOUND_STMT, "AST_COMPOUND_STMT") \
    /** Functions. */ \
    take(AST_FUNCTION_DECL, "AST_FUNCTION_DECL") \
    take(AST_FUNCTION_CALL, "AST_FUNCTION_CALL") \
    /** Implicit type conversion. */ \
    take(AST_IMPLICIT_CAST, "AST_IMPLICIT_CAST")

enum ast_type {
    map(__take_enum)
};

really_inline static const char *ast_type_to_string(int t)
{
    static const char *names[] = { map(__take_string) };
    return names[t];
}

#undef __take_string
#undef __take_enum
#undef map

#endif // FCC_FRONTEND_AST_AST_TYPE_H