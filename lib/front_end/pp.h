/* pp.h - Preprocessor.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_FRONTEND_PP_H
#define FCC_FRONTEND_PP_H

#include "util/compiler.h"
#include "util/vector.h"
#include "front_end/tok.h"

typedef vector_t(struct token) tokens_t;

void pp_init();
void pp_deinit();
tokens_t *pp(const char *filename);

void pp_add_include_path(const char *path);

#endif // FCC_FRONTEND_PP_H