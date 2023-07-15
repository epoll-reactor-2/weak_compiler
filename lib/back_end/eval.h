/* eval.h - Weak language IR interpreter.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_BACKEND_EVAL_H
#define WEAK_COMPILER_BACKEND_EVAL_H

#include <stdint.h>

struct ir_node;

int32_t eval(struct ir_node *ir);

#endif // WEAK_COMPILER_BACKEND_EVAL_H