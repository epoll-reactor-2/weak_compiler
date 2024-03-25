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
    case '=':  return T_ASSIGN;
    case '^':  return T_BIT_XOR;
    case '&':  return T_BIT_AND;
    case '|':  return T_BIT_OR;
    case '>':  return T_GT;
    case '<':  return T_LT;
    case '+':  return T_PLUS;
    case '-':  return T_MINUS;
    case '*':  return T_STAR;
    case '/':  return T_SLASH;
    case '%':  return T_MOD;
    case '.':  return T_DOT;
    case ',':  return T_COMMA;
    case ':':  return T_COLON;
    case ';':  return T_SEMICOLON;
    case '!':  return T_EXCLAMATION;
    case '[':  return T_OPEN_BRACKET;
    case ']':  return T_CLOSE_BRACKET;
    case '{':  return T_OPEN_BRACE;
    case '}':  return T_CLOSE_BRACE;
    case '(':  return T_OPEN_PAREN;
    case ')':  return T_CLOSE_PAREN;
    case '\n': return T_NEWLINE;
    default:
        fcc_unreachable("Unknown character operation (char: `%c`).", c);
    }
}