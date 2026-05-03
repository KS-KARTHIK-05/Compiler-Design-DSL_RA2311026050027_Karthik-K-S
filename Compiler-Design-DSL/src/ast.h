/*
 * ast.h  –  Abstract Syntax Tree (AST) node definitions
 *
 * All AST nodes share a NodeType tag. The union/struct below
 * carries the data needed per node kind.
 */

#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Node kinds ──────────────────────────────────────────── */
typedef enum {
    /* Literals */
    NODE_NUM,           /* integer constant          */
    NODE_BOOL,          /* boolean constant          */
    NODE_STR,           /* string literal            */
    NODE_ID,            /* variable reference        */

    /* Declarations */
    NODE_DECL_INT,      /* int x;                    */
    NODE_DECL_BOOL,     /* bool x;                   */
    NODE_ASSIGN,        /* x = expr;                 */

    /* Arithmetic */
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_MOD,
    NODE_NEG,           /* unary minus               */

    /* Relational */
    NODE_EQ,
    NODE_NEQ,
    NODE_LT,
    NODE_GT,
    NODE_LEQ,
    NODE_GEQ,

    /* Logical */
    NODE_AND,
    NODE_OR,
    NODE_NOT,

    /* Control flow */
    NODE_IF,            /* if (cond) then [else]     */
    NODE_WHILE,         /* while (cond) body         */

    /* I/O */
    NODE_PRINT,         /* print(expr)               */

    /* Sequence */
    NODE_BLOCK,         /* { stmt* }                 */
    NODE_STMT_LIST,     /* linked list of stmts      */

    /* Program root */
    NODE_PROGRAM
} NodeType;

/* ── Forward declaration ─────────────────────────────────── */
typedef struct ASTNode ASTNode;

/* ── AST Node ────────────────────────────────────────────── */
struct ASTNode {
    NodeType type;

    /* ── Payload (varies by node type) ─────────────────── */
    int    ival;        /* NODE_NUM / NODE_BOOL          */
    char  *sval;        /* NODE_ID / NODE_STR / decl name*/

    /* ── Children ──────────────────────────────────────── */
    ASTNode *left;      /* first child / condition       */
    ASTNode *right;     /* second child / then-branch    */
    ASTNode *extra;     /* third child (else-branch)     */
    ASTNode *next;      /* sibling in statement list     */

    int line;           /* source line (for error msgs)  */
};

/* ── Constructor ─────────────────────────────────────────── */
static inline ASTNode *new_node(NodeType type) {
    ASTNode *n = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (!n) { perror("calloc"); exit(1); }
    n->type = type;
    return n;
}

/* ── Convenience constructors ────────────────────────────── */
static inline ASTNode *make_num(int val) {
    ASTNode *n = new_node(NODE_NUM);
    n->ival = val;
    return n;
}

static inline ASTNode *make_bool(int val) {
    ASTNode *n = new_node(NODE_BOOL);
    n->ival = val;
    return n;
}

static inline ASTNode *make_id(const char *name) {
    ASTNode *n = new_node(NODE_ID);
    n->sval = strdup(name);
    return n;
}

static inline ASTNode *make_str(const char *s) {
    ASTNode *n = new_node(NODE_STR);
    n->sval = strdup(s);
    return n;
}

static inline ASTNode *make_binop(NodeType op, ASTNode *l, ASTNode *r) {
    ASTNode *n = new_node(op);
    n->left  = l;
    n->right = r;
    return n;
}

static inline ASTNode *make_unop(NodeType op, ASTNode *operand) {
    ASTNode *n = new_node(op);
    n->left = operand;
    return n;
}

/* ── Pretty-printer (for debugging / docs output) ────────── */
static const char *node_name(NodeType t) {
    switch (t) {
        case NODE_NUM:        return "NUM";
        case NODE_BOOL:       return "BOOL";
        case NODE_STR:        return "STR";
        case NODE_ID:         return "ID";
        case NODE_DECL_INT:   return "DECL_INT";
        case NODE_DECL_BOOL:  return "DECL_BOOL";
        case NODE_ASSIGN:     return "ASSIGN";
        case NODE_ADD:        return "ADD";
        case NODE_SUB:        return "SUB";
        case NODE_MUL:        return "MUL";
        case NODE_DIV:        return "DIV";
        case NODE_MOD:        return "MOD";
        case NODE_NEG:        return "NEG";
        case NODE_EQ:         return "EQ";
        case NODE_NEQ:        return "NEQ";
        case NODE_LT:         return "LT";
        case NODE_GT:         return "GT";
        case NODE_LEQ:        return "LEQ";
        case NODE_GEQ:        return "GEQ";
        case NODE_AND:        return "AND";
        case NODE_OR:         return "OR";
        case NODE_NOT:        return "NOT";
        case NODE_IF:         return "IF";
        case NODE_WHILE:      return "WHILE";
        case NODE_PRINT:      return "PRINT";
        case NODE_BLOCK:      return "BLOCK";
        case NODE_STMT_LIST:  return "STMT_LIST";
        case NODE_PROGRAM:    return "PROGRAM";
        default:              return "UNKNOWN";
    }
}

void ast_print(ASTNode *node, int depth);

/* ── Free AST ────────────────────────────────────────────── */
void ast_free(ASTNode *node);

#endif /* AST_H */
