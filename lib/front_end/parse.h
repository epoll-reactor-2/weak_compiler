/* parse.h - Syntax analyzer.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_FRONTEND_PARSE_PARSE_H
#define FCC_FRONTEND_PARSE_PARSE_H

#include "front_end/tok.h"
#include "util/compiler.h"

struct ast_node;

void pp_init();
void pp_deinit();
void pp_add_include_path(const char *path);

wur
struct ast_node *parse(const char *filename);

#endif // FCC_FRONTEND_PARSE_PARSE_H