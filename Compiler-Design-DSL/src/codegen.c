/*
 * codegen.c  –  Three-Address Code (TAC) generation
 */

#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Create a new TAC list ───────────────────────────────── */
TACList *taclist_new(void) {
    TACList *tac = (TACList *)calloc(1, sizeof(TACList));
    if (!tac) { perror("calloc"); exit(1); }
    return tac;
}

/* ── Emit one TAC instruction ────────────────────────────── */
static void emit(TACList *tac,
                 const char *op,
                 const char *arg1, const char *arg2,
                 const char *result) {
    TACInstr *ins = (TACInstr *)calloc(1, sizeof(TACInstr));
    strncpy(ins->op,     op     ? op     : "", MAX_TAC_OP  - 1);
    strncpy(ins->arg1,   arg1   ? arg1   : "", MAX_TAC_ARG - 1);
    strncpy(ins->arg2,   arg2   ? arg2   : "", MAX_TAC_ARG - 1);
    strncpy(ins->result, result ? result : "", MAX_TAC_ARG - 1);

    if (!tac->head) tac->head = tac->tail = ins;
    else { tac->tail->next = ins; tac->tail = ins; }
}

/* ── Fresh temporary variable name ──────────────────────── */
static void new_temp(TACList *tac, char *buf) {
    sprintf(buf, "t%d", tac->temp_count++);
}

/* ── Fresh label name ────────────────────────────────────── */
static void new_label(TACList *tac, char *buf) {
    sprintf(buf, "L%d", tac->label_count++);
}

/* ── Recursive code generation; returns result name ─────── */
static void gen(ASTNode *node, TACList *tac, char *result_out);

static void gen(ASTNode *node, TACList *tac, char *result_out) {
    if (!node) { if (result_out) result_out[0] = '\0'; return; }

    char t1[MAX_TAC_ARG], t2[MAX_TAC_ARG], tmp[MAX_TAC_ARG];

    switch (node->type) {

        /* ── Literals ─────────────────────────────────────── */
        case NODE_NUM:
            new_temp(tac, tmp);
            sprintf(t1, "%d", node->ival);
            emit(tac, "=", t1, "", tmp);
            if (result_out) strncpy(result_out, tmp, MAX_TAC_ARG - 1);
            break;

        case NODE_BOOL:
            new_temp(tac, tmp);
            emit(tac, "=", node->ival ? "true" : "false", "", tmp);
            if (result_out) strncpy(result_out, tmp, MAX_TAC_ARG - 1);
            break;

        case NODE_STR:
            new_temp(tac, tmp);
            snprintf(t1, MAX_TAC_ARG, "\"%s\"",
                     node->sval ? node->sval : "");
            emit(tac, "=", t1, "", tmp);
            if (result_out) strncpy(result_out, tmp, MAX_TAC_ARG - 1);
            break;

        case NODE_ID:
            if (result_out)
                strncpy(result_out,
                        node->sval ? node->sval : "?",
                        MAX_TAC_ARG - 1);
            break;

        /* ── Declarations ─────────────────────────────────── */
        case NODE_DECL_INT:
            emit(tac, "DECL_INT", node->sval, "", "");
            if (result_out) result_out[0] = '\0';
            break;

        case NODE_DECL_BOOL:
            emit(tac, "DECL_BOOL", node->sval, "", "");
            if (result_out) result_out[0] = '\0';
            break;

        /* ── Assignment ───────────────────────────────────── */
        case NODE_ASSIGN:
            gen(node->right, tac, t1);
            emit(tac, "=", t1, "", node->left->sval);
            if (result_out) strncpy(result_out, node->left->sval, MAX_TAC_ARG-1);
            break;

        /* ── Binary Arithmetic ────────────────────────────── */
        case NODE_ADD: case NODE_SUB:
        case NODE_MUL: case NODE_DIV: case NODE_MOD: {
            const char *op_str =
                node->type == NODE_ADD ? "+" :
                node->type == NODE_SUB ? "-" :
                node->type == NODE_MUL ? "*" :
                node->type == NODE_DIV ? "/" : "%";
            gen(node->left,  tac, t1);
            gen(node->right, tac, t2);
            new_temp(tac, tmp);
            emit(tac, op_str, t1, t2, tmp);
            if (result_out) strncpy(result_out, tmp, MAX_TAC_ARG - 1);
            break;
        }

        /* ── Unary Negation ───────────────────────────────── */
        case NODE_NEG:
            gen(node->left, tac, t1);
            new_temp(tac, tmp);
            emit(tac, "NEG", t1, "", tmp);
            if (result_out) strncpy(result_out, tmp, MAX_TAC_ARG - 1);
            break;

        /* ── Relational Operators ─────────────────────────── */
        case NODE_EQ:  case NODE_NEQ:
        case NODE_LT:  case NODE_GT:
        case NODE_LEQ: case NODE_GEQ: {
            const char *op_str =
                node->type == NODE_EQ  ? "==" :
                node->type == NODE_NEQ ? "!=" :
                node->type == NODE_LT  ? "<"  :
                node->type == NODE_GT  ? ">"  :
                node->type == NODE_LEQ ? "<=" : ">=";
            gen(node->left,  tac, t1);
            gen(node->right, tac, t2);
            new_temp(tac, tmp);
            emit(tac, op_str, t1, t2, tmp);
            if (result_out) strncpy(result_out, tmp, MAX_TAC_ARG - 1);
            break;
        }

        /* ── Logical Operators ────────────────────────────── */
        case NODE_AND:
            gen(node->left,  tac, t1);
            gen(node->right, tac, t2);
            new_temp(tac, tmp);
            emit(tac, "&&", t1, t2, tmp);
            if (result_out) strncpy(result_out, tmp, MAX_TAC_ARG - 1);
            break;

        case NODE_OR:
            gen(node->left,  tac, t1);
            gen(node->right, tac, t2);
            new_temp(tac, tmp);
            emit(tac, "||", t1, t2, tmp);
            if (result_out) strncpy(result_out, tmp, MAX_TAC_ARG - 1);
            break;

        case NODE_NOT:
            gen(node->left, tac, t1);
            new_temp(tac, tmp);
            emit(tac, "NOT", t1, "", tmp);
            if (result_out) strncpy(result_out, tmp, MAX_TAC_ARG - 1);
            break;

        /* ── If Statement ─────────────────────────────────── */
        case NODE_IF: {
            char label_else[MAX_TAC_ARG];
            char label_end[MAX_TAC_ARG];
            new_label(tac, label_else);
            new_label(tac, label_end);

            /* evaluate condition */
            gen(node->left, tac, t1);
            emit(tac, "IF_FALSE", t1, label_else, "");

            /* then-branch */
            gen(node->right, tac, NULL);

            if (node->extra) {
                /* jump over else */
                emit(tac, "GOTO", label_end, "", "");
            }

            /* else label */
            emit(tac, "LABEL", label_else, "", "");

            if (node->extra) {
                gen(node->extra, tac, NULL);
                emit(tac, "LABEL", label_end, "", "");
            }

            if (result_out) result_out[0] = '\0';
            break;
        }

        /* ── While Statement ──────────────────────────────── */
        case NODE_WHILE: {
            char label_start[MAX_TAC_ARG];
            char label_end[MAX_TAC_ARG];
            new_label(tac, label_start);
            new_label(tac, label_end);

            emit(tac, "LABEL", label_start, "", "");
            gen(node->left, tac, t1);              /* condition */
            emit(tac, "IF_FALSE", t1, label_end, "");
            gen(node->right, tac, NULL);            /* body */
            emit(tac, "GOTO", label_start, "", "");
            emit(tac, "LABEL", label_end, "", "");

            if (result_out) result_out[0] = '\0';
            break;
        }

        /* ── Print Statement ──────────────────────────────── */
        case NODE_PRINT:
            gen(node->left, tac, t1);
            emit(tac, "PRINT", t1, "", "");
            if (result_out) result_out[0] = '\0';
            break;

        /* ── Block / Statement List ───────────────────────── */
        case NODE_BLOCK:
        case NODE_STMT_LIST:
        case NODE_PROGRAM: {
            ASTNode *stmt = node->left;
            while (stmt) {
                gen(stmt, tac, NULL);
                stmt = stmt->next;
            }
            if (result_out) result_out[0] = '\0';
            break;
        }

        default:
            if (result_out) result_out[0] = '\0';
            break;
    }
}

/* ── Public entry point ──────────────────────────────────── */
void codegen(ASTNode *root, TACList *tac, SymTab *st) {
    (void)st;   /* reserved for future type-annotated code gen */
    gen(root, tac, NULL);
}

/* ── Print TAC to file/stdout ────────────────────────────── */
void taclist_print(TACList *tac, FILE *out) {
    fprintf(out, "\n");
    fprintf(out, "  ┌─────────────────────────────────────────────────────┐\n");
    fprintf(out, "  │          THREE-ADDRESS CODE (TAC) OUTPUT             │\n");
    fprintf(out, "  ├──────┬────────────┬──────────────┬──────────────────┤\n");
    fprintf(out, "  │ %-4s │ %-10s │ %-12s │ %-16s │\n",
            "No.", "Operator", "Arg1 / Arg2", "Result");
    fprintf(out, "  ├──────┼────────────┼──────────────┼──────────────────┤\n");

    int n = 0;
    for (TACInstr *ins = tac->head; ins; ins = ins->next, n++) {

        /* Format arg display */
        char args[64] = {0};
        if (ins->arg2[0])
            snprintf(args, sizeof(args), "%s, %s", ins->arg1, ins->arg2);
        else if (ins->arg1[0])
            snprintf(args, sizeof(args), "%s", ins->arg1);

        fprintf(out, "  │ %-4d │ %-10s │ %-12s │ %-16s │\n",
                n, ins->op, args, ins->result);
    }
    fprintf(out, "  └──────┴────────────┴──────────────┴──────────────────┘\n");
    fprintf(out, "  Total instructions: %d\n\n", n);
}

/* ── Free TAC list ───────────────────────────────────────── */
void taclist_free(TACList *tac) {
    TACInstr *ins = tac->head;
    while (ins) {
        TACInstr *tmp = ins->next;
        free(ins);
        ins = tmp;
    }
    free(tac);
}
