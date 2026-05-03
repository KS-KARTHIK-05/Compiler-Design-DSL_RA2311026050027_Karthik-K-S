%{
/*
 * lexer.l  –  Lexical Analyser (Flex)
 * DSL: Simple Imperative Language (if / while / print / arithmetic)
 *
 * Tokens produced:
 *   Keywords : IF ELSE WHILE PRINT INT BOOL TRUE FALSE
 *   Literals : NUMBER IDENTIFIER STRING_LIT
 *   Operators: PLUS MINUS MUL DIV MOD ASSIGN
 *   Relational: EQ NEQ LT GT LEQ GEQ
 *   Logical  : AND OR NOT
 *   Punctuation: LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"   /* tokens from Bison */

int line_num = 1;         /* track line numbers for error messages */

void lex_error(const char *msg);
%}

/* ── Options ──────────────────────────────────────────────── */
%option noyywrap
%option yylineno

/* ── Definitions ─────────────────────────────────────────── */
DIGIT        [0-9]
LETTER       [a-zA-Z_]
ALNUM        [a-zA-Z0-9_]
WS           [ \t\r]+

%%

 /* ── Whitespace & Comments ─────────────────────────────── */
{WS}                { /* skip */ }
\n                  { line_num++; }
"//"[^\n]*          { /* single-line comment, skip */ }
"/*"([^*]|\*[^/])*"*/" { /* multi-line comment */
                        for (char *p = yytext; *p; p++)
                            if (*p == '\n') line_num++;
                    }

 /* ── Keywords ──────────────────────────────────────────── */
"if"                { return IF; }
"else"              { return ELSE; }
"while"             { return WHILE; }
"print"             { return PRINT; }
"int"               { return INT; }
"bool"              { return BOOL; }
"true"              { yylval.ival = 1; return TRUE_LIT; }
"false"             { yylval.ival = 0; return FALSE_LIT; }

 /* ── Relational Operators (before single-char ops) ─────── */
"=="                { return EQ; }
"!="                { return NEQ; }
"<="                { return LEQ; }
">="                { return GEQ; }
"<"                 { return LT; }
">"                 { return GT; }

 /* ── Logical ────────────────────────────────────────────── */
"&&"                { return AND; }
"||"                { return OR; }
"!"                 { return NOT; }

 /* ── Assignment ─────────────────────────────────────────── */
"="                 { return ASSIGN; }

 /* ── Arithmetic Operators ───────────────────────────────── */
"+"                 { return PLUS; }
"-"                 { return MINUS; }
"*"                 { return MUL; }
"/"                 { return DIV; }
"%"                 { return MOD; }

 /* ── Punctuation ────────────────────────────────────────── */
"("                 { return LPAREN; }
")"                 { return RPAREN; }
"{"                 { return LBRACE; }
"}"                 { return RBRACE; }
";"                 { return SEMICOLON; }
","                 { return COMMA; }

 /* ── Integer Literal ────────────────────────────────────── */
{DIGIT}+            {
                        yylval.ival = atoi(yytext);
                        return NUMBER;
                    }

 /* ── Identifier ─────────────────────────────────────────── */
{LETTER}{ALNUM}*    {
                        yylval.sval = strdup(yytext);
                        return IDENTIFIER;
                    }

 /* ── String Literal ─────────────────────────────────────── */
\"[^\"]*\"          {
                        /* strip surrounding quotes */
                        yylval.sval = strndup(yytext + 1, strlen(yytext) - 2);
                        return STRING_LIT;
                    }

 /* ── Unknown character ──────────────────────────────────── */
.                   {
                        fprintf(stderr,
                            "[LEXER ERROR] Line %d: Unknown character '%s'\n",
                            line_num, yytext);
                    }

%%

void lex_error(const char *msg) {
    fprintf(stderr, "[LEXER ERROR] Line %d: %s\n", line_num, msg);
}
