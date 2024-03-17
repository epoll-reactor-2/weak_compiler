/* ir_bin.h - Read/write IR in binary format.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_MIDDLE_END_IR_BIN_H
#define FCC_MIDDLE_END_IR_BIN_H

struct ir_unit;

void ir_write_binary(struct ir_unit *unit, const char *filename);

struct ir_unit ir_read_binary(const char *filename);

#endif /* FCC_MIDDLE_END_IR_BIN_H */