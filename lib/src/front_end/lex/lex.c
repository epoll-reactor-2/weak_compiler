/* lex.c - flex-based lexical analyzer.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/lex/lex.h"
#include <stdio.h>
#include <stdlib.h>

void lex_consume_token(tok_t *tok)
{
    /// \todo: Collect tokens to some sort of vector.
    printf("lineno: %d, colno: %d\n", tok->line_no, tok->col_no);
    fflush(stdout);
    if (tok->data) {
        free(tok->data);
    }
}