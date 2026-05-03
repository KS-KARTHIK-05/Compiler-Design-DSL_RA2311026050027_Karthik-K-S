/*
 * codegen.h / codegen.c  –  Intermediate Code Generation
 *
 * Generates Three-Address Code (TAC) from the AST.
 *
 * TAC format:
 *   result = op1 operator op2    (binary)
 *   result = operator op1        (unary)
 *   result = op1                 (copy)
 *   PARAM op1                    (print argument)
 *   PRINT op1
 *   LABEL Lxx:
 *   GOTO  Lxx
 *   IF_FALSE op1 GOTO Lxx
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── TAC instruction ─────────────────────────────────────── */
#define MAX_TAC_OP   16
#define MAX_TAC_ARG  32

typedef struct TACInstr {
    char op[MAX_TAC_OP];      /* operator / mnemonic  */
    char arg1[MAX_TAC_ARG];
    char arg2[MAX_TAC_ARG];
    char result[MAX_TAC_ARG];
    struct TACInstr *next;
} TACInstr;

typedef struct {
    TACInstr *head;
    TACInstr *tail;
    int       temp_count;
    int       label_count;
} TACList;

/* ── API ─────────────────────────────────────────────────── */
TACList *taclist_new(void);
void     codegen(ASTNode *root, TACList *tac, SymTab *st);
void     taclist_print(TACList *tac, FILE *out);
void     taclist_free(TACList *tac);

#endif /* CODEGEN_H */
