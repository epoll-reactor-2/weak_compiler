/* parse.h - Syntax analyzer.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_FRONTEND_PARSE_PARSE_H
#define FCC_FRONTEND_PARSE_PARSE_H

#include "front_end/lex/tok.h"
#include "util/compiler.h"

struct ast_node;

wur
struct ast_node *parse(const struct token *begin, const struct token *end);

#endif // FCC_FRONTEND_PARSE_PARSE_H