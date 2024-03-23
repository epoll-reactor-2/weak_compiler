/* tok.c - String conversion function for the token enum.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "front_end/tok.h"
#include "util/unreachable.h"

enum token_type tok_char_to_tok(char c)
{
    switch (c) {
    case '=': return TOK_ASSIGN;
    case '^': return TOK_BIT_XOR;
    case '&': return TOK_BIT_AND;
    case '|': return TOK_BIT_OR;
    case '>': return TOK_GT;
    case '<': return TOK_LT;
    case '+': return TOK_PLUS;
    case '-': return TOK_MINUS;
    case '*': return TOK_STAR;
    case '/': return TOK_SLASH;
    case '%': return TOK_MOD;
    case '.': return TOK_DOT;
    case ',': return TOK_COMMA;
    case ':': return TOK_COLON;
    case ';': return TOK_SEMICOLON;
    case '!': return TOK_EXCLAMATION;
    case '[': return TOK_OPEN_BRACKET;
    case ']': return TOK_CLOSE_BRACKET;
    case '{': return TOK_OPEN_BRACE;
    case '}': return TOK_CLOSE_BRACE;
    case '(': return TOK_OPEN_PAREN;
    case ')': return TOK_CLOSE_PAREN;
    default:
        fcc_unreachable("Unknown character operation (char: `%c`).", c);
    }
}