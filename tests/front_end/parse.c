/* parse.c - Test case for parser.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/ast_dump.h"
#include "utils/test_utils.h"
#include <unistd.h>

void *diag_error_memstream = NULL;
void *diag_warn_memstream = NULL;

void __parse_test(unused const char *path, const char *filename, FILE *out_stream)
{
    struct ast_node *ast = gen_ast(filename);
    puts("");
    ast_dump(out_stream, ast);
    ast_dump(stdout, ast);
    ast_node_cleanup(ast);
}

int parse_test(const char *path, const char *filename)
{
    return compare_with_comment(path, filename, __parse_test);
}

void pp_local()
{
    char cwd[384];
    if (!getcwd(cwd, 383)) {
        perror("getcwd()");
        exit(-1);
    }

    char pp[512] = {0};
    snprintf(pp, 511, "%s/inputs/parser", cwd);
    pp_add_include_path(pp);
}

int main()
{
    pp_init();
    pp_local();
    int rc = do_on_each_file("parser", parse_test);
    pp_deinit();
    return rc;
}
