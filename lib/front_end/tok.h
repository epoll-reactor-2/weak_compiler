/* tok.h - List of all token types.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_FRONTEND_T_H
#define FCC_FRONTEND_T_H

#include "util/compiler.h"
#include <stdbool.h>
#include <stdint.h>

#define __take_enum(x, y) x,
#define __take_string(x, y) y,

#define map(take) \
    take(T_NULL__, "") \
    /* Keywords. */ \
    take(T_ALIGNOF, "alignof") \
    take(T_AUTO, "auto") \
    take(T_BREAK, "break") \
    take(T_CASE, "case") \
    take(T_CHAR, "char") \
    take(T_CONST, "const") \
    take(T_CONTINUE, "continue") \
    take(T_DEFAULT, "default") \
    take(T_DO, "do") \
    take(T_DOUBLE, "double") \
    take(T_ELSE, "else") \
    take(T_ENUM, "enum") \
    take(T_EXTERN, "extern") \
    take(T_FLOAT, "float") \
    take(T_FOR, "for") \
    take(T_GOTO, "goto") \
    take(T_IF, "if") \
    take(T_INLINE, "inline") \
    take(T_INT, "int") \
    take(T_LONG, "long") \
    take(T_REGISTER, "register") \
    take(T_RESTRICT, "restrict") \
    take(T_RETURN, "return") \
    take(T_SHORT, "short") \
    take(T_SIGNED, "signed") \
    take(T_SIZEOF, "sizeof") \
    take(T_STATIC, "static") \
    take(T_STRUCT, "struct") \
    take(T_SWITCH, "switch") \
    take(T_TYPEDEF, "typedef") \
    take(T_UNION, "union") \
    take(T_UNSIGNED, "unsigned") \
    take(T_VOID, "void") \
    take(T_VOLATILE, "volatile") \
    take(T_WHILE, "while") \
    take(T_ALIGNAS, "_Alignas") \
    take(T_ATOMIC, "_Atomic") \
    take(T_BOOL, "_Bool") \
    take(T_COMPLEX, "_Complex") \
    take(T_GENERIC, "_Generic") \
    take(T_IMAGINARY, "_Imaginary") \
    take(T_NORETURN, "_Noreturn") \
    take(T_STATIC_ASSERT, "_Static_assert") \
    take(T_THREAD_LOCAL, "_Thread_local") \
    /* 6.10 if-group:
       \
        `if` keyword is present. */ \
    take(T_IFDEF, "ifdef") \
    take(T_IFNDEF, "ifndef") \
    /* 6.10 elif-groups:
       \
        `else` keyword is present. */ \
    take(T_ELIF, "elif") \
    /* 6.10 endif-line */ \
    take(T_ENDIF, "endif") \
    /* 6.10 control-line */ \
    take(T_INCLUDE, "include") \
    take(T_DEFINE, "define") \
    take(T_UNDEF, "undef") \
    take(T_LINE, "line") \
    take(T_LINE, "error") \
    take(T_LINE, "pragma") \
    \
    take(T_OPEN_BRACKET, "[") \
    take(T_CLOSE_BRACKET, "]") \
    take(T_OPEN_PAREN, "(") \
    take(T_CLOSE_PAREN, ")") \
    take(T_OPEN_BRACE, "{") \
    take(T_CLOSE_BRACE, "}") \
    take(T_DOT, ".") \
    take(T_ARROW, "->") \
    take(T_INC, "++") \
    take(T_DEC, "--") \
    take(T_BIT_AND, "&") \
    take(T_STAR, "*") \
    take(T_PLUS, "+") \
    take(T_MINUS, "-") \
    take(T_TILDE, "~") \
    take(T_EXCLAMATION, "!") \
    take(T_SLASH, "/") \
    take(T_MOD, "%") \
    take(T_SHL, "<<") \
    take(T_SHR, ">>") \
    take(T_LT, "<") \
    take(T_GT, ">") \
    take(T_LE, "<=") \
    take(T_GE, ">=") \
    take(T_EQ, "==") \
    take(T_NEQ, "!=") \
    take(T_BIT_XOR, "^") \
    take(T_BIT_OR, "|") \
    take(T_AND, "&&") \
    take(T_OR, "||") \
    take(T_QUESTION_MARK, "?") \
    take(T_COLON, ":") \
    take(T_SEMICOLON, ";") \
    take(T_ELLIPSIS, "...") \
    take(T_ASSIGN, "=") \
    take(T_MUL_ASSIGN, "*=") \
    take(T_DIV_ASSIGN, "/=") \
    take(T_MOD_ASSIGN, "%=") \
    take(T_PLUS_ASSIGN, "+=") \
    take(T_MINUS_ASSIGN, "-=") \
    take(T_SHL_ASSIGN, "<<=") \
    take(T_SHR_ASSIGN, ">>=") \
    take(T_BIT_AND_ASSIGN, "&=") \
    take(T_BIT_XOR_ASSIGN, "^=") \
    take(T_BIT_OR_ASSIGN, "|=") \
    take(T_COMMA, ",") \
    take(T_HASH, "#") \
    take(T_HASH_HASH, "##") \
    take(T_INT_LITERAL, "<integer literal>") \
    take(T_FLOAT_LITERAL, "<float literal>") \
    take(T_STRING_LITERAL, "<string literal>") \
    take(T_CHAR_LITERAL, "<char literal>") \
    take(T_SYM, "<symbol>")

enum token_type {
    map(__take_enum)
};

really_inline static const char *tok_to_string(int t)
{
    static const char *names[] = { map(__take_string) };
    return names[t];
}

#undef __take_string
#undef __take_enum
#undef map

struct token {
    char            *data;
    enum token_type  type;
    uint16_t         line_no;
    uint16_t         col_no;
};

enum token_type tok_char_to_tok(char c);

wur really_inline bool tok_is(const struct token *tok, char symbol)
{
    return tok->type == tok_char_to_tok(symbol);
}

#endif // FCC_FRONTEND_T_H