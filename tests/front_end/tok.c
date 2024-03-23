/* tok.c - Test case for token.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/tok.h"
#include "utils/test_utils.h"

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

int main()
{
    struct token token1 = {
        .data    = "",
        .type    = T_ASSIGN,
        .line_no = 0U,
        .col_no  = 0U
    };
    ASSERT_TRUE(tok_is(&token1, '='));
    ASSERT_EQ(tok_to_string(T_EXCLAMATION), "!");
    ASSERT_EQ(tok_to_string(T_STAR), "*");
    ASSERT_EQ(tok_to_string(T_STATIC_ASSERT), "_Static_assert");
    ASSERT_EQ(tok_char_to_tok('+'), T_PLUS);
    ASSERT_EQ(tok_char_to_tok('*'), T_STAR);
    ASSERT_EQ(tok_char_to_tok('&'), T_BIT_AND);
    ASSERT_EQ(tok_char_to_tok('|'), T_BIT_OR);
    ASSERT_EQ(tok_char_to_tok('['), T_OPEN_BRACKET);
    ASSERT_EQ(tok_char_to_tok(']'), T_CLOSE_BRACKET);
}