/*
 * semantic.h / semantic.c  –  Semantic Analysis
 *
 * Checks performed:
 *   1. Undeclared variable usage
 *   2. Duplicate variable declarations
 *   3. Type mismatch in assignments
 *   4. Type mismatch in if/while conditions (must be bool or int)
 *   5. Division by zero (constant folding lite)
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtab.h"

int  semantic_check(ASTNode *root, SymTab *st);
VarType infer_type(ASTNode *node, SymTab *st);

#endif /* SEMANTIC_H */
