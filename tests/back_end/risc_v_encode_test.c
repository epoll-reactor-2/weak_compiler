/* risc_v_encode_test.c - Tests for RISC-V encoding.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "back_end/back_end.h"
#include "back_end/risc_v.h"
#include "utils/test_utils.h"
#include <stddef.h>
#include <stdio.h>

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

struct codegen_output output = {0};

 #define __swap(x,y) do {   \
    typeof(x) _x = x;       \
    typeof(y) _y = y;       \
    x = _y;                 \
    y = _x;                 \
 } while(0)

void dump_bytes(const void *bytes, uint64_t len)
{
    const uint8_t *mem = bytes;

    for (uint64_t i = 0; i < len; ++i)
        printf("%02x", mem[i]);
}

/* Just for convenient tests with
   https://luplab.gitlab.io/rvcodecjs */
void be_to_le(void *bytes, uint64_t len)
{
    uint8_t *mem = bytes;

    for (uint64_t i = len; i >= len / 2; --i)
        __swap(mem[i], mem[len - i - 1]);
}

int match(uint64_t len, const char *bytes)
{
    ASSERT_TRUE(output.text.count > 0);

    if (output.text.count != len) {
        printf(
            "RISC-V encoding failed: %ld vs %ld bytes were encoded\n",
            output.text.count, len
        );
    }

    be_to_le(output.text.data, len);

    if (memcmp(output.text.data, bytes, len)) {
        printf("RISC-V encoding failed\n ");
        dump_bytes(output.text.data, output.text.count);
        printf(" got,\n ");
        dump_bytes(bytes, len);
        printf(" expected\n");
        ASSERT_TRUE(0);
    }

    vector_clear(output.text);
}

int main()
{
    hashmap_init(&output.fn_offsets, 1);
    back_end_init(&output);

    back_end_native_addiw(risc_v_reg_t0, risc_v_reg_t0, 1);
    match(4, "\x00\x12\x82\x9b");

    back_end_native_and(risc_v_reg_a2, risc_v_reg_a3, risc_v_reg_a4);
    match(4, "\x00\xe6\xf6\x33");

    back_end_native_addi(risc_v_reg_a2, risc_v_reg_a3, 0xfffff);
    match(8, "\x00\x06\x06\x13\xff\xf6\x86\x13");
    //        \               \
    //         \               addi a2, a3, 0x7ffff
    //          \
    //           addi a2, a2, 0x80000
    ASSERT_EQ(0xfffff, 0x7ffff + 0x80000);

    back_end_native_lb(risc_v_reg_t0, risc_v_reg_t1, 2047);
    match(4, "\x7f\xf3\x02\x83");

    back_end_native_lbu(risc_v_reg_t0, risc_v_reg_t1, 2047);
    match(4, "\x7f\xf3\x42\x83");

    back_end_native_lh(risc_v_reg_t0, risc_v_reg_t1, 2047);
    match(4, "\x7f\xf3\x12\x83");

    back_end_native_lhu(risc_v_reg_t0, risc_v_reg_t1, 2047);
    match(4, "\x7f\xf3\x52\x83");

    back_end_native_lw(risc_v_reg_t0, risc_v_reg_t1, 2047);
    match(4, "\x7f\xf3\x22\x83");

    back_end_native_lwu(risc_v_reg_t0, risc_v_reg_t1, 2047);
    match(4, "\x7f\xf3\x62\x83");

    back_end_native_ld(risc_v_reg_t0, risc_v_reg_t1, 2047);
    match(4, "\x7f\xf3\x32\x83");

    back_end_native_ret();
    match(4, "\x00\x00\x80\x67");
}
