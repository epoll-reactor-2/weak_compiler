/* gen.c - Tests for IR generator.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/lex/lex.h"
#include "front_end/parse/parse.h"
#include "middle_end/ir/dump.h"
#include "util/diagnostic.h"
#include "utils/test_utils.h"
#include <stdio.h>

extern int yylex_destroy();

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

bool ir_test(const char *path, const char *filename)
{
    (void) filename;

    bool    ok               = 1;
    char   *expected         = NULL;
    char   *generated        = NULL;
    size_t  _                = 0;
    FILE   *expected_stream  = open_memstream(&expected, &_);
    FILE   *generated_stream = open_memstream(&generated, &_);

    if (expected_stream == NULL) {
        perror("open_memstream()");
        return 0;
    }

    if (!setjmp(weak_fatal_error_buf)) {
        struct ir_unit *ir = gen_ir(path);

        extract_assertion_comment(yyin, expected_stream);

        ir_dump_unit(generated_stream, ir);
        fflush(generated_stream);
        ir_unit_cleanup(ir);
        
        if (strcmp(expected, generated) != 0) {
            printf("IR mismatch:\n%s\ngot,\n%s\nexpected\n", generated, expected);
            ok = 0;
            goto exit;
        }
        printf("Success!\n");
    } else {
        /// Error, will be printed in main.
        return 0;
    }

exit:
    yylex_destroy();
    fclose(expected_stream);
    fclose(generated_stream);
    free(expected);
    free(generated);

    return ok;
}

int main()
{
    int ret             = 0;
    char *err_buf       = NULL;
    char *warn_buf      = NULL;
    size_t err_buf_len  = 0;
    size_t warn_buf_len = 0;

    diag_error_memstream = open_memstream(&err_buf, &err_buf_len);
    diag_warn_memstream = open_memstream(&warn_buf, &warn_buf_len);

    if (!do_on_each_file("/test_inputs/ir_gen", ir_test)) {
        ret = -1;

        if (err_buf) {
            fputs(err_buf, stderr);
            return ret;
        }

        if (warn_buf)
            fputs(warn_buf, stderr);
    }

    fclose(diag_error_memstream);
    fclose(diag_warn_memstream);
    free(err_buf);
    free(warn_buf);
    return ret;
}