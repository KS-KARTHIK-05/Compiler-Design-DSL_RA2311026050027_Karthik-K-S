/*
 * semantic.c  –  Semantic Analysis implementation
 */

#include "semantic.h"
#include <stdio.h>

static int error_count = 0;

#define SEM_ERROR(fmt, ...) \
    do { fprintf(stderr, "[SEMANTIC ERROR] " fmt "\n", ##__VA_ARGS__); \
         error_count++; } while(0)

#define SEM_WARN(fmt, ...) \
    do { fprintf(stderr, "[SEMANTIC WARN]  " fmt "\n", ##__VA_ARGS__); } while(0)

/* ── Infer the type of an expression node ─────────────────── */
VarType infer_type(ASTNode *node, SymTab *st) {
    if (!node) return TYPE_VOID;

    switch (node->type) {
        case NODE_NUM:    return TYPE_INT;
        case NODE_BOOL:   return TYPE_BOOL;
        case NODE_STR:    return TYPE_STRING;

        case NODE_ID: {
            Symbol *sym = symtab_lookup(st, node->sval);
            if (!sym) return TYPE_UNKNOWN;
            return sym->type;
        }

        /* Arithmetic → int */
        case NODE_ADD: case NODE_SUB:
        case NODE_MUL: case NODE_DIV:
        case NODE_MOD: case NODE_NEG:
            return TYPE_INT;

        /* Relational / Logical → bool */
        case NODE_EQ:  case NODE_NEQ:
        case NODE_LT:  case NODE_GT:
        case NODE_LEQ: case NODE_GEQ:
        case NODE_AND: case NODE_OR:
        case NODE_NOT:
            return TYPE_BOOL;

        default:
            return TYPE_UNKNOWN;
    }
}

/* ── Walk the AST performing semantic checks ──────────────── */
static void walk(ASTNode *node, SymTab *st) {
    if (!node) return;

    switch (node->type) {

        /* ── Variable declaration ────────────────────────── */
        case NODE_DECL_INT:
        case NODE_DECL_BOOL: {
            VarType vt = (node->type == NODE_DECL_INT) ? TYPE_INT : TYPE_BOOL;
            int rc = symtab_insert(st, node->sval, vt, node->line);
            if (rc < 0)
                SEM_ERROR("Duplicate declaration of variable '%s'", node->sval);
            break;
        }

        /* ── Assignment ──────────────────────────────────── */
        case NODE_ASSIGN: {
            const char *name = node->left->sval;
            Symbol *sym = symtab_lookup(st, name);
            if (!sym) {
                SEM_ERROR("Assignment to undeclared variable '%s'", name);
            } else {
                symtab_mark_used(st, name);
                walk(node->right, st);
                VarType rhs_type = infer_type(node->right, st);
                if (rhs_type != TYPE_UNKNOWN &&
                    sym->type != rhs_type) {
                    SEM_ERROR("Type mismatch: cannot assign %s to variable "
                              "'%s' (declared as %s)",
                              type_name(rhs_type), name,
                              type_name(sym->type));
                }
            }
            return; /* children already handled */
        }

        /* ── Identifier use ──────────────────────────────── */
        case NODE_ID: {
            Symbol *sym = symtab_lookup(st, node->sval);
            if (!sym)
                SEM_ERROR("Use of undeclared variable '%s'", node->sval);
            else
                symtab_mark_used(st, node->sval);
            break;
        }

        /* ── Division – check for divide-by-zero constant ── */
        case NODE_DIV: {
            walk(node->left,  st);
            walk(node->right, st);
            if (node->right && node->right->type == NODE_NUM &&
                node->right->ival == 0) {
                SEM_ERROR("Division by zero detected (constant RHS = 0)");
            }
            return;
        }

        /* ── If statement ────────────────────────────────── */
        case NODE_IF: {
            walk(node->left, st);   /* condition */
            VarType ct = infer_type(node->left, st);
            if (ct == TYPE_STRING)
                SEM_ERROR("'if' condition must be int or bool, got string");
            walk(node->right, st);  /* then-branch */
            walk(node->extra, st);  /* else-branch (may be NULL) */
            return;
        }

        /* ── While statement ─────────────────────────────── */
        case NODE_WHILE: {
            walk(node->left, st);   /* condition */
            VarType ct = infer_type(node->left, st);
            if (ct == TYPE_STRING)
                SEM_ERROR("'while' condition must be int or bool, got string");
            walk(node->right, st);  /* body */
            return;
        }

        /* ── Print – allow any type ──────────────────────── */
        case NODE_PRINT: {
            walk(node->left, st);
            break;
        }

        default:
            break;
    }

    /* ── Recurse into children ──────────────────────────────── */
    walk(node->left,  st);
    walk(node->right, st);
    walk(node->extra, st);
    walk(node->next,  st);
}

/* ── Entry point ──────────────────────────────────────────── */
int semantic_check(ASTNode *root, SymTab *st) {
    error_count = 0;
    walk(root, st);

    if (error_count == 0)
        printf("[SEMANTIC] Analysis passed with 0 errors.\n");
    else
        fprintf(stderr, "[SEMANTIC] Analysis failed with %d error(s).\n",
                error_count);

    return error_count;
}
