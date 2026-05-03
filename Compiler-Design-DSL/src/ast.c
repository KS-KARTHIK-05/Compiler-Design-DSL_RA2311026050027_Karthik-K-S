/*
 * ast.c  –  AST utility implementations (print + free)
 */

#include "ast.h"

/* ── Print AST as indented tree ──────────────────────────── */
void ast_print(ASTNode *node, int depth) {
    if (!node) return;

    /* Indent */
    for (int i = 0; i < depth; i++) printf("  ");

    /* Print node label */
    switch (node->type) {
        case NODE_NUM:
            printf("[NUM: %d]\n", node->ival);
            break;
        case NODE_BOOL:
            printf("[BOOL: %s]\n", node->ival ? "true" : "false");
            break;
        case NODE_STR:
            printf("[STR: \"%s\"]\n", node->sval ? node->sval : "");
            break;
        case NODE_ID:
            printf("[ID: %s]\n", node->sval ? node->sval : "?");
            break;
        case NODE_DECL_INT:
            printf("[DECL_INT: %s]\n", node->sval ? node->sval : "?");
            break;
        case NODE_DECL_BOOL:
            printf("[DECL_BOOL: %s]\n", node->sval ? node->sval : "?");
            break;
        default:
            printf("[%s]\n", node_name(node->type));
            break;
    }

    /* Recurse into children */
    ast_print(node->left,  depth + 1);
    ast_print(node->right, depth + 1);
    ast_print(node->extra, depth + 1);
    ast_print(node->next,  depth);     /* siblings at same depth */
}

/* ── Free AST recursively ────────────────────────────────── */
void ast_free(ASTNode *node) {
    if (!node) return;
    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->extra);
    ast_free(node->next);
    if (node->sval) free(node->sval);
    free(node);
}
