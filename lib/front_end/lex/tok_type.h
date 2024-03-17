/* tok_type.h - List of all token types.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_FRONTEND_LEX_TOK_TYPE_H
#define FCC_FRONTEND_LEX_TOK_TYPE_H

#include "util/compiler.h"

#define __take_enum(x, y) x,
#define __take_string(x, y) y,

#define map(take) \
    take(TOK_ALIGNOF, "alignof") \
    take(TOK_AUTO, "auto") \
    take(TOK_BREAK, "break") \
    take(TOK_CASE, "case") \
    take(TOK_CHAR, "char") \
    take(TOK_CONST, "const") \
    take(TOK_CONTINUE, "continue") \
    take(TOK_DEFAULT, "default") \
    take(TOK_DO, "do") \
    take(TOK_DOUBLE, "double") \
    take(TOK_ELSE, "else") \
    take(TOK_ENUM, "enum") \
    take(TOK_EXTERN, "extern") \
    take(TOK_FLOAT, "float") \
    take(TOK_FOR, "for") \
    take(TOK_GOTO, "goto") \
    take(TOK_IF, "if") \
    take(TOK_INLINE, "inline") \
    take(TOK_INT, "int") \
    take(TOK_LONG, "long") \
    take(TOK_REGISTER, "register") \
    take(TOK_RESTRICT, "restrict") \
    take(TOK_RETURN, "return") \
    take(TOK_SHORT, "short") \
    take(TOK_SIGNED, "signed") \
    take(TOK_SIZEOF, "sizeof") \
    take(TOK_STATIC, "static") \
    take(TOK_STRUCT, "struct") \
    take(TOK_SWITCH, "switch") \
    take(TOK_TYPEDEF, "typedef") \
    take(TOK_UNION, "union") \
    take(TOK_UNSIGNED, "unsigned") \
    take(TOK_VOID, "void") \
    take(TOK_VOLATILE, "volatile") \
    take(TOK_WHILE, "while") \
    take(TOK_ALIGNAS, "_Alignas") \
    take(TOK_ATOMIC, "_Atomic") \
    take(TOK_BOOL, "_Bool") \
    take(TOK_COMPLEX, "_Complex") \
    take(TOK_GENERIC, "_Generic") \
    take(TOK_IMAGINARY, "_Imaginary") \
    take(TOK_NORETURN, "_Noreturn") \
    take(TOK_STATIC_ASSERT, "_Static_assert") \
    take(TOK_THREAD_LOCAL, "_Thread_local") \
    take(TOK_OPEN_BRACKET, "[") \
    take(TOK_CLOSE_BRACKET, "]") \
    take(TOK_OPEN_PAREN, "(") \
    take(TOK_CLOSE_PAREN, ")") \
    take(TOK_OPEN_BRACE, "{") \
    take(TOK_CLOSE_BRACE, "}") \
    take(TOK_DOT, ".") \
    take(TOK_ARROW, "->") \
    take(TOK_INC, "++") \
    take(TOK_DEC, "--") \
    take(TOK_BIT_AND, "&") \
    take(TOK_STAR, "*") \
    take(TOK_PLUS, "+") \
    take(TOK_MINUS, "-") \
    take(TOK_TILDE, "~") \
    take(TOK_EXCLAMATION, "!") \
    take(TOK_SLASH, "/") \
    take(TOK_MOD, "%") \
    take(TOK_SHL, "<<") \
    take(TOK_SHR, ">>") \
    take(TOK_LT, "<") \
    take(TOK_GT, ">") \
    take(TOK_LE, "<=") \
    take(TOK_GE, ">=") \
    take(TOK_EQ, "==") \
    take(TOK_NEQ, "!=") \
    take(TOK_BIT_XOR, "^") \
    take(TOK_BIT_OR, "|") \
    take(TOK_AND, "&&") \
    take(TOK_OR, "||") \
    take(TOK_QUESTION_MARK, "?") \
    take(TOK_COLON, ":") \
    take(TOK_SEMICOLON, ";") \
    take(TOK_ELLIPSIS, "...") \
    take(TOK_ASSIGN, "=") \
    take(TOK_MUL_ASSIGN, "*=") \
    take(TOK_DIV_ASSIGN, "/=") \
    take(TOK_MOD_ASSIGN, "%=") \
    take(TOK_PLUS_ASSIGN, "+=") \
    take(TOK_MINUS_ASSIGN, "-=") \
    take(TOK_SHL_ASSIGN, "<<=") \
    take(TOK_SHR_ASSIGN, ">>=") \
    take(TOK_BIT_AND_ASSIGN, "&=") \
    take(TOK_BIT_XOR_ASSIGN, "^=") \
    take(TOK_BIT_OR_ASSIGN, "|=") \
    take(TOK_COMMA, ",") \
    take(TOK_HASH, "#") \
    take(TOK_HASH_HASH, "##") \
    take(TOK_LESS_COLON, "<:") \
    take(TOK_COLON_GREATER, ":>") \
    take(TOK_LESS_PERCENT, "<%") \
    take(TOK_PERCENT_GREATER, "%>") \
    take(TOK_PERCENT_COLON, "%:") \
    take(TOK_PERCENT_PERCENT, "%:%:") \
    take(TOK_INT_LITERAL, "<integer literal>") \
    take(TOK_FLOAT_LITERAL, "<float literal>") \
    take(TOK_STRING_LITERAL, "<string literal>") \
    take(TOK_CHAR_LITERAL, "<char literal>") \
    take(TOK_SYM, "<symbol>")

enum token_type {
    map(__take_enum)
};

really_inline static const char *tok_to_string(int t)
{
    static const char *names[] = { map(__take_string) };
    return names[t];
}

enum token_type tok_char_to_tok(char c);

#undef __take_string
#undef __take_enum
#undef map

#endif // FCC_FRONTEND_LEX_TOK_TYPE_H