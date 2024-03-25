/* ast_type.c - Test case AST enums.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/ast_type.h"
#include "utils/test_utils.h"

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

int main()
{
    ASSERT_EQ(ast_type_to_string(AST_FOR_STMT), "AST_FOR_STMT");
    ASSERT_EQ(ast_type_to_string(AST_FOR_RANGE_STMT), "AST_FOR_RANGE_STMT");
    ASSERT_EQ(ast_type_to_string(AST_WHILE_STMT), "AST_WHILE_STMT");
}