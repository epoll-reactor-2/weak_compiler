/* grammar.lex - Language acceptable tokens specification.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

%{

#include "front_end/tok.h"

extern void lex_token(struct token *tok);

int yycolumn = 1;

#define YY_USER_ACTION                                                   \
    lex_lineno = prev_yylineno;                                          \
    lex_colno = yycolumn;                                                \
    if (yylineno == prev_yylineno) {                                     \
        yycolumn += yyleng;                                              \
    } else {                                                             \
        for (yycolumn = 1; yytext[yyleng - yycolumn]; ++yycolumn) {}     \
        prev_yylineno = yylineno;                                        \
    }

#define LEX_WORD(tok_type) do {                                          \
    struct token t = {                                                   \
        .data    = strdup(yytext),                                       \
        .type    = tok_type,                                             \
        .line_no = lex_lineno,                                           \
        .col_no  = lex_colno                                             \
    };                                                                   \
    lex_token(&t);                                                       \
    return tok_type;                                                     \
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
    lex_token(&t);                                                       \
    return tok_type;                                                     \
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
    lex_token(&t);                                                       \
    return tok_type;                                                     \
} while (0);

%}
%option noyywrap nounput noinput
%option yylineno

%x C_COMMENT

%%
 int lex_lineno, lex_colno;
 int prev_yylineno = yylineno;

"/*"            { BEGIN(C_COMMENT); }
<C_COMMENT>"*/" { BEGIN(INITIAL); }
<C_COMMENT>\n   {}
<C_COMMENT>.    {}
(\ |\t|\v)      {}

-?[0-9]+               LEX_WORD(T_INT_LITERAL)
-?[0-9]+\.[0-9]+       LEX_WORD(T_FLOAT_LITERAL)
\"(([^\"\\]|\\.)*)\"   LEX_QUOTED_LITERAL(T_STRING_LITERAL)
\'.\'                  LEX_QUOTED_LITERAL(T_CHAR_LITERAL)

[_a-zA-Z][_a-zA-Z0-9]*\( LEX_WORD(T_MACRO)

"alignof"              LEX_WORD(T_ALIGNOF)
"auto"                 LEX_WORD(T_AUTO)
"break"                LEX_WORD(T_BREAK)
"case"                 LEX_WORD(T_CASE)
"char"                 LEX_WORD(T_CHAR)
"const"                LEX_WORD(T_CONST)
"continue"             LEX_WORD(T_CONTINUE)
"default"              LEX_WORD(T_DEFAULT)
"do"                   LEX_WORD(T_DO)
"double"               LEX_WORD(T_DOUBLE)
"else"                 LEX_WORD(T_ELSE)
"enum"                 LEX_WORD(T_ENUM)
"extern"               LEX_WORD(T_EXTERN)
"float"                LEX_WORD(T_FLOAT)
"for"                  LEX_WORD(T_FOR)
"goto"                 LEX_WORD(T_GOTO)
"if"                   LEX_WORD(T_IF)
"inline"               LEX_WORD(T_INLINE)
"int"                  LEX_WORD(T_INT)
"long"                 LEX_WORD(T_LONG)
"register"             LEX_WORD(T_REGISTER)
"restrict"             LEX_WORD(T_RESTRICT)
"return"               LEX_WORD(T_RETURN)
"short"                LEX_WORD(T_SHORT)
"signed"               LEX_WORD(T_SIGNED)
"sizeof"               LEX_WORD(T_SIZEOF)
"static"               LEX_WORD(T_STATIC)
"struct"               LEX_WORD(T_STRUCT)
"switch"               LEX_WORD(T_SWITCH)
"typedef"              LEX_WORD(T_TYPEDEF)
"union"                LEX_WORD(T_UNION)
"unsigned"             LEX_WORD(T_UNSIGNED)
"void"                 LEX_WORD(T_VOID)
"volatile"             LEX_WORD(T_VOLATILE)
"while"                LEX_WORD(T_WHILE)
"_Alignas"             LEX_WORD(T_ALIGNAS)
"_Atomic"              LEX_WORD(T_ATOMIC)
"_Bool"                LEX_WORD(T_BOOL)
"_Complex"             LEX_WORD(T_COMPLEX)
"_Generic"             LEX_WORD(T_GENERIC)
"_Imaginary"           LEX_WORD(T_IMAGINARY)
"_Noreturn"            LEX_WORD(T_NORETURN)
"_Static_assert"       LEX_WORD(T_STATIC_ASSERT)
"_Thread_local"        LEX_WORD(T_THREAD_LOCAL)
"ifdef"                LEX_WORD(T_IFDEF)
"ifndef"               LEX_WORD(T_IFNDEF)
"elif"                 LEX_WORD(T_ELIF)
"endif"                LEX_WORD(T_ENDIF)
"include"              LEX_WORD(T_INCLUDE)
"define"               LEX_WORD(T_DEFINE)
"undef"                LEX_WORD(T_UNDEF)
"line"                 LEX_WORD(T_LINE)
"error"                LEX_WORD(T_ERROR)
"pragma"               LEX_WORD(T_PRAGMA)

"["                    LEX_OP(T_OPEN_BRACKET)
"]"                    LEX_OP(T_CLOSE_BRACKET)
"<:"                   LEX_OP(T_OPEN_BRACKET)
":>"                   LEX_OP(T_CLOSE_BRACKET)
"{"                    LEX_OP(T_OPEN_BRACE)
"}"                    LEX_OP(T_CLOSE_BRACE)
"<%"                   LEX_OP(T_OPEN_BRACE)
"%>"                   LEX_OP(T_CLOSE_BRACE)
"("                    LEX_OP(T_OPEN_PAREN)
")"                    LEX_OP(T_CLOSE_PAREN)
"."                    LEX_OP(T_DOT)
"++"                   LEX_OP(T_INC)
"->"                   LEX_OP(T_ARROW)
"--"                   LEX_OP(T_DEC)
"&"                    LEX_OP(T_BIT_AND)
"*"                    LEX_OP(T_STAR)
"+"                    LEX_OP(T_PLUS)
"-"                    LEX_OP(T_MINUS)
"~"                    LEX_OP(T_TILDE)
"!"                    LEX_OP(T_EXCLAMATION)
"/"                    LEX_OP(T_SLASH)
"%"                    LEX_OP(T_MOD)
"<<"                   LEX_OP(T_SHL)
">>"                   LEX_OP(T_SHR)
"<"                    LEX_OP(T_LT)
">"                    LEX_OP(T_GT)
"<="                   LEX_OP(T_LE)
">="                   LEX_OP(T_GE)
"=="                   LEX_OP(T_EQ)
"!="                   LEX_OP(T_NEQ)
"^"                    LEX_OP(T_BIT_XOR)
"|"                    LEX_OP(T_BIT_OR)
"&&"                   LEX_OP(T_AND)
"||"                   LEX_OP(T_OR)
"?"                    LEX_OP(T_QUESTION_MARK)
":"                    LEX_OP(T_COLON)
";"                    LEX_OP(T_SEMICOLON)
"..."                  LEX_OP(T_ELLIPSIS)
"="                    LEX_OP(T_ASSIGN)
"*="                   LEX_OP(T_MUL_ASSIGN)
"/="                   LEX_OP(T_DIV_ASSIGN)
"%="                   LEX_OP(T_MOD_ASSIGN)
"+="                   LEX_OP(T_PLUS_ASSIGN)
"-="                   LEX_OP(T_MINUS_ASSIGN)
"<<="                  LEX_OP(T_SHL_ASSIGN)
">>="                  LEX_OP(T_SHR_ASSIGN)
"&="                   LEX_OP(T_BIT_AND_ASSIGN)
"^="                   LEX_OP(T_BIT_XOR_ASSIGN)
"|="                   LEX_OP(T_BIT_OR_ASSIGN)
","                    LEX_OP(T_COMMA)
"#"                    LEX_OP(T_HASH)
"##"                   LEX_OP(T_HASH_HASH)
"\\"                   LEX_OP(T_BACKSLASH)
"%:"                   LEX_OP(T_HASH)
"%:%:"                 LEX_OP(T_HASH_HASH)
"\n"                   LEX_OP(T_NEWLINE)

[_a-zA-Z][_a-zA-Z0-9]* LEX_WORD(T_SYM)

.                      { fprintf(stderr, "Illegal token `%s`\n", yytext);
                         fflush (stderr);
                         __builtin_trap();
                       }

%%