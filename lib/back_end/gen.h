/* gen.h - Code generator.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_BACKEND_GEN_H
#define WEAK_COMPILER_BACKEND_GEN_H

#include "back_end/elf.h"

struct ast_node;

void back_end_gen(struct ast_node *ast);

#endif // WEAK_COMPILER_BACKEND_GEN_H