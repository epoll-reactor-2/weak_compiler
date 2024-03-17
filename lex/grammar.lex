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

#define LEX_WORD(tok_type) do {                                          \
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
#define LEX_QUOTED_LITERAL(tok_type) do {                                \
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
#define LEX_OP(tok_type) do {                                            \
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

-?[0-9]+               LEX_WORD(TOK_INT_LITERAL)
-?[0-9]+\.[0-9]+       LEX_WORD(TOK_FLOAT_LITERAL)
\"(([^\"\\]|\\.)*)\"   LEX_QUOTED_LITERAL(TOK_STRING_LITERAL)
\'.\'                  LEX_QUOTED_LITERAL(TOK_CHAR_LITERAL)

"alignof"              LEX_WORD(TOK_ALIGNOF)
"auto"                 LEX_WORD(TOK_AUTO)
"break"                LEX_WORD(TOK_BREAK)
"case"                 LEX_WORD(TOK_CASE)
"char"                 LEX_WORD(TOK_CHAR)
"const"                LEX_WORD(TOK_CONST)
"continue"             LEX_WORD(TOK_CONTINUE)
"default"              LEX_WORD(TOK_DEFAULT)
"do"                   LEX_WORD(TOK_DO)
"double"               LEX_WORD(TOK_DOUBLE)
"else"                 LEX_WORD(TOK_ELSE)
"enum"                 LEX_WORD(TOK_ENUM)
"extern"               LEX_WORD(TOK_EXTERN)
"float"                LEX_WORD(TOK_FLOAT)
"for"                  LEX_WORD(TOK_FOR)
"goto"                 LEX_WORD(TOK_GOTO)
"if"                   LEX_WORD(TOK_IF)
"inline"               LEX_WORD(TOK_INLINE)
"int"                  LEX_WORD(TOK_INT)
"long"                 LEX_WORD(TOK_LONG)
"register"             LEX_WORD(TOK_REGISTER)
"restrict"             LEX_WORD(TOK_RESTRICT)
"return"               LEX_WORD(TOK_RETURN)
"short"                LEX_WORD(TOK_SHORT)
"signed"               LEX_WORD(TOK_SIGNED)
"sizeof"               LEX_WORD(TOK_SIZEOF)
"static"               LEX_WORD(TOK_STATIC)
"struct"               LEX_WORD(TOK_STRUCT)
"switch"               LEX_WORD(TOK_SWITCH)
"typedef"              LEX_WORD(TOK_TYPEDEF)
"union"                LEX_WORD(TOK_UNION)
"unsigned"             LEX_WORD(TOK_UNSIGNED)
"void"                 LEX_WORD(TOK_VOID)
"volatile"             LEX_WORD(TOK_VOLATILE)
"while"                LEX_WORD(TOK_WHILE)
"_Alignas"             LEX_WORD(TOK_ALIGNAS)
"_Atomic"              LEX_WORD(TOK_ATOMIC)
"_Bool"                LEX_WORD(TOK_BOOL)
"_Complex"             LEX_WORD(TOK_COMPLEX)
"_Generic"             LEX_WORD(TOK_GENERIC)
"_Imaginary"           LEX_WORD(TOK_IMAGINARY)
"_Noreturn"            LEX_WORD(TOK_NORETURN)
"_Static_assert"       LEX_WORD(TOK_STATIC_ASSERT)
"_Thread_local"        LEX_WORD(TOK_THREAD_LOCAL)

"["                    LEX_OP(TOK_OPEN_BRACKET)
"]"                    LEX_OP(TOK_CLOSE_BRACKET)
"("                    LEX_OP(TOK_OPEN_PAREN)
")"                    LEX_OP(TOK_CLOSE_PAREN)
"{"                    LEX_OP(TOK_OPEN_BRACE)
"}"                    LEX_OP(TOK_CLOSE_BRACE)
"."                    LEX_OP(TOK_DOT)
"++"                   LEX_OP(TOK_INC)
"->"                   LEX_OP(TOK_ARROW)
"--"                   LEX_OP(TOK_DEC)
"&"                    LEX_OP(TOK_BIT_AND)
"*"                    LEX_OP(TOK_STAR)
"+"                    LEX_OP(TOK_PLUS)
"-"                    LEX_OP(TOK_MINUS)
"~"                    LEX_OP(TOK_TILDE)
"!"                    LEX_OP(TOK_EXCLAMATION)
"/"                    LEX_OP(TOK_SLASH)
"%"                    LEX_OP(TOK_MOD)
"<<"                   LEX_OP(TOK_SHL)
">>"                   LEX_OP(TOK_SHR)
"<"                    LEX_OP(TOK_LT)
">"                    LEX_OP(TOK_GT)
"<="                   LEX_OP(TOK_LE)
">="                   LEX_OP(TOK_GE)
"=="                   LEX_OP(TOK_EQ)
"!="                   LEX_OP(TOK_NEQ)
"^"                    LEX_OP(TOK_BIT_XOR)
"|"                    LEX_OP(TOK_BIT_OR)
"&&"                   LEX_OP(TOK_AND)
"||"                   LEX_OP(TOK_OR)
"?"                    LEX_OP(TOK_QUESTION_MARK)
":"                    LEX_OP(TOK_COLON)
";"                    LEX_OP(TOK_SEMICOLON)
"..."                  LEX_OP(TOK_ELLIPSIS)
"="                    LEX_OP(TOK_ASSIGN)
"*="                   LEX_OP(TOK_MUL_ASSIGN)
"/="                   LEX_OP(TOK_DIV_ASSIGN)
"%="                   LEX_OP(TOK_MOD_ASSIGN)
"+="                   LEX_OP(TOK_PLUS_ASSIGN)
"-="                   LEX_OP(TOK_MINUS_ASSIGN)
"<<="                  LEX_OP(TOK_SHL_ASSIGN)
">>="                  LEX_OP(TOK_SHR_ASSIGN)
"&="                   LEX_OP(TOK_BIT_AND_ASSIGN)
"^="                   LEX_OP(TOK_BIT_XOR_ASSIGN)
"|="                   LEX_OP(TOK_BIT_OR_ASSIGN)
","                    LEX_OP(TOK_COMMA)
"#"                    LEX_OP(TOK_HASH)
"##"                   LEX_OP(TOK_HASH_HASH)
"<:"                   LEX_OP(TOK_LESS_COLON)
":>"                   LEX_OP(TOK_COLON_GREATER)
"<%"                   LEX_OP(TOK_LESS_PERCENT)
"%>"                   LEX_OP(TOK_PERCENT_GREATER)
"%:"                   LEX_OP(TOK_PERCENT_COLON)
"%:%:"                 LEX_OP(TOK_PERCENT_PERCENT)

[_a-zA-Z][_a-zA-Z0-9]* LEX_WORD(TOK_SYM)

.                      { fprintf(stderr, "Illegal token `%s`\n", yytext);
                         fflush (stderr);
                         __builtin_trap();
                       }

%%