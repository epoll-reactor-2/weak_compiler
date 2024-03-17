/* grammar.lex - Language acceptable tokens specification.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

%{

#include "front_end/lex/tok.h"

int yycolumn = 1;

#define YY_USER_ACTION                                                   \
  lex_lineno = prev_yylineno;                                            \
  lex_colno = yycolumn;                                                  \
  if (yylineno == prev_yylineno) {                                       \
      yycolumn += yyleng;                                                \
  } else {                                                               \
    for (yycolumn = 1; yytext[yyleng - yycolumn] != '\n'; ++yycolumn) {} \
    prev_yylineno = yylineno;                                            \
  }

extern void lex_consume_token(struct token *tok);

#define LEX_CONSUME_WORD(tok_type) do {                                  \
    struct token t = {                                                   \
        .data    = strdup(yytext),                                       \
        .type    = tok_type,                                             \
        .line_no = lex_lineno,                                           \
        .col_no  = lex_colno                                             \
    };                                                                   \
    lex_consume_token(&t);                                               \
} while (0);

/* Don't include quotes to match. Lex has no lookahead in their regular
   expression engine, so we emulate it by hand. */
#define LEX_CONSUME_QUOTED_LITERAL(tok_type) do {                        \
    ++yytext;                                                            \
    struct token t = {                                                   \
        .data    = strdup(yytext),                                       \
        .type    = tok_type,                                             \
        .line_no = lex_lineno,                                           \
        .col_no  = lex_colno                                             \
    };                                                                   \
    --yytext;                                                            \
    size_t data_len = strlen(t.data);                                    \
    if (data_len > 0)                                                    \
        t.data[data_len - 1] = '\0';                                     \
    lex_consume_token(&t);                                               \
} while (0);

/* Operator can have no data, since all information about it contained
   in type. */
#define LEX_CONSUME_OPERATOR(tok_type) do {                              \
    struct token t = {                                                   \
        .data    = NULL,                                                 \
        .type    = tok_type,                                             \
        .line_no = lex_lineno,                                           \
        .col_no  = lex_colno                                             \
    };                                                                   \
    lex_consume_token(&t);                                               \
} while (0);

%}
%option noyywrap nounput noinput
%option yylineno

%%
 int lex_lineno, lex_colno;
 int prev_yylineno = yylineno;
   
\/\/.*\n               /* One-line comment. */
\/\*.*\*\/             /* Multi-line comment. */
[[:space:]]            /* Ignore whitespace. */

-?[0-9]+               LEX_CONSUME_WORD(TOK_INT_LITERAL)
-?[0-9]+\.[0-9]+       LEX_CONSUME_WORD(TOK_FLOAT_LITERAL)
\"(([^\"\\]|\\.)*)\"   LEX_CONSUME_QUOTED_LITERAL(TOK_STRING_LITERAL)
\'.\'                  LEX_CONSUME_QUOTED_LITERAL(TOK_CHAR_LITERAL)

"alignof"              LEX_CONSUME_WORD(TOK_ALIGNOF)
"auto"                 LEX_CONSUME_WORD(TOK_AUTO)
"break"                LEX_CONSUME_WORD(TOK_BREAK)
"case"                 LEX_CONSUME_WORD(TOK_CASE)
"char"                 LEX_CONSUME_WORD(TOK_CHAR)
"const"                LEX_CONSUME_WORD(TOK_CONST)
"continue"             LEX_CONSUME_WORD(TOK_CONTINUE)
"default"              LEX_CONSUME_WORD(TOK_DEFAULT)
"do"                   LEX_CONSUME_WORD(TOK_DO)
"double"               LEX_CONSUME_WORD(TOK_DOUBLE)
"else"                 LEX_CONSUME_WORD(TOK_ELSE)
"enum"                 LEX_CONSUME_WORD(TOK_ENUM)
"extern"               LEX_CONSUME_WORD(TOK_EXTERN)
"float"                LEX_CONSUME_WORD(TOK_FLOAT)
"for"                  LEX_CONSUME_WORD(TOK_FOR)
"goto"                 LEX_CONSUME_WORD(TOK_GOTO)
"if"                   LEX_CONSUME_WORD(TOK_IF)
"inline"               LEX_CONSUME_WORD(TOK_INLINE)
"int"                  LEX_CONSUME_WORD(TOK_INT)
"long"                 LEX_CONSUME_WORD(TOK_LONG)
"register"             LEX_CONSUME_WORD(TOK_REGISTER)
"restrict"             LEX_CONSUME_WORD(TOK_RESTRICT)
"return"               LEX_CONSUME_WORD(TOK_RETURN)
"short"                LEX_CONSUME_WORD(TOK_SHORT)
"signed"               LEX_CONSUME_WORD(TOK_SIGNED)
"sizeof"               LEX_CONSUME_WORD(TOK_SIZEOF)
"static"               LEX_CONSUME_WORD(TOK_STATIC)
"struct"               LEX_CONSUME_WORD(TOK_STRUCT)
"switch"               LEX_CONSUME_WORD(TOK_SWITCH)
"typedef"              LEX_CONSUME_WORD(TOK_TYPEDEF)
"union"                LEX_CONSUME_WORD(TOK_UNION)
"unsigned"             LEX_CONSUME_WORD(TOK_UNSIGNED)
"void"                 LEX_CONSUME_WORD(TOK_VOID)
"volatile"             LEX_CONSUME_WORD(TOK_VOLATILE)
"while"                LEX_CONSUME_WORD(TOK_WHILE)
"_Alignas"             LEX_CONSUME_WORD(TOK_ALIGNAS)
"_Atomic"              LEX_CONSUME_WORD(TOK_ATOMIC)
"_Bool"                LEX_CONSUME_WORD(TOK_BOOL)
"_Complex"             LEX_CONSUME_WORD(TOK_COMPLEX)
"_Generic"             LEX_CONSUME_WORD(TOK_GENERIC)
"_Imaginary"           LEX_CONSUME_WORD(TOK_IMAGINARY)
"_Noreturn"            LEX_CONSUME_WORD(TOK_NORETURN)
"_Static_assert"       LEX_CONSUME_WORD(TOK_STATIC_ASSERT)
"_Thread_local"        LEX_CONSUME_WORD(TOK_THREAD_LOCAL)

"bool"                 LEX_CONSUME_WORD(TOK_BOOL)
"break"                LEX_CONSUME_WORD(TOK_BREAK)
"char"                 LEX_CONSUME_WORD(TOK_CHAR)
"continue"             LEX_CONSUME_WORD(TOK_CONTINUE)
"do"                   LEX_CONSUME_WORD(TOK_DO)
"else"                 LEX_CONSUME_WORD(TOK_ELSE)
"float"                LEX_CONSUME_WORD(TOK_FLOAT)
"for"                  LEX_CONSUME_WORD(TOK_FOR)
"if"                   LEX_CONSUME_WORD(TOK_IF)
"int"                  LEX_CONSUME_WORD(TOK_INT)
"return"               LEX_CONSUME_WORD(TOK_RETURN)
"struct"               LEX_CONSUME_WORD(TOK_STRUCT)
"void"                 LEX_CONSUME_WORD(TOK_VOID)
"while"                LEX_CONSUME_WORD(TOK_WHILE)

"="                    LEX_CONSUME_OPERATOR(TOK_ASSIGN)
"/="                   LEX_CONSUME_OPERATOR(TOK_DIV_ASSIGN)
"%="                   LEX_CONSUME_OPERATOR(TOK_MOD_ASSIGN)
"+="                   LEX_CONSUME_OPERATOR(TOK_PLUS_ASSIGN)
"-="                   LEX_CONSUME_OPERATOR(TOK_MINUS_ASSIGN)
">>="                  LEX_CONSUME_OPERATOR(TOK_SHR_ASSIGN)
"<<="                  LEX_CONSUME_OPERATOR(TOK_SHL_ASSIGN)
"&="                   LEX_CONSUME_OPERATOR(TOK_BIT_AND_ASSIGN)
"|="                   LEX_CONSUME_OPERATOR(TOK_BIT_OR_ASSIGN)
"^="                   LEX_CONSUME_OPERATOR(TOK_BIT_XOR_ASSIGN)
"&&"                   LEX_CONSUME_OPERATOR(TOK_AND)
"||"                   LEX_CONSUME_OPERATOR(TOK_OR)
"&"                    LEX_CONSUME_OPERATOR(TOK_BIT_AND)
"|"                    LEX_CONSUME_OPERATOR(TOK_BIT_OR)
"=="                   LEX_CONSUME_OPERATOR(TOK_EQ)
"!="                   LEX_CONSUME_OPERATOR(TOK_NEQ)
">"                    LEX_CONSUME_OPERATOR(TOK_GT)
"<"                    LEX_CONSUME_OPERATOR(TOK_LT)
">="                   LEX_CONSUME_OPERATOR(TOK_GE)
"<="                   LEX_CONSUME_OPERATOR(TOK_LE)
"<<"                   LEX_CONSUME_OPERATOR(TOK_SHL)
">>"                   LEX_CONSUME_OPERATOR(TOK_SHR)
"+"                    LEX_CONSUME_OPERATOR(TOK_PLUS)
"-"                    LEX_CONSUME_OPERATOR(TOK_MINUS)
"*"                    LEX_CONSUME_OPERATOR(TOK_STAR)
"/"                    LEX_CONSUME_OPERATOR(TOK_SLASH)
"%"                    LEX_CONSUME_OPERATOR(TOK_MOD)
"++"                   LEX_CONSUME_OPERATOR(TOK_INC)
"--"                   LEX_CONSUME_OPERATOR(TOK_DEC)
"."                    LEX_CONSUME_OPERATOR(TOK_DOT)
","                    LEX_CONSUME_OPERATOR(TOK_COMMA)
":"                    LEX_CONSUME_OPERATOR(TOK_COLON)
";"                    LEX_CONSUME_OPERATOR(TOK_SEMICOLON)
"!"                    LEX_CONSUME_OPERATOR(TOK_EXCLAMATION)
"^"                    LEX_CONSUME_OPERATOR(TOK_BIT_XOR)
"["                    LEX_CONSUME_OPERATOR(TOK_OPEN_BRACKET)
"]"                    LEX_CONSUME_OPERATOR(TOK_CLOSE_BRACKET)
"("                    LEX_CONSUME_OPERATOR(TOK_OPEN_PAREN)
")"                    LEX_CONSUME_OPERATOR(TOK_CLOSE_PAREN)
"{"                    LEX_CONSUME_OPERATOR(TOK_OPEN_BRACE)
"}"                    LEX_CONSUME_OPERATOR(TOK_CLOSE_BRACE)

[_a-zA-Z][_a-zA-Z0-9]* LEX_CONSUME_WORD(TOK_SYM)

.                      { fprintf(stderr, "Illegal token `%s`\n", yytext);
                         fflush (stderr);
                         __builtin_trap();
                       }

%%