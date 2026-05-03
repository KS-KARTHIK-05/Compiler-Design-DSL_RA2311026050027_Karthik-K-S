/*
 * ============================================================
 *  TENSOR'26 Compiler Design Assignment - HoT Task
 *  Problem 3: Grammar Processing – Left Recursion Risk Detection
 *
 *  Tool    : C (mimics Pandas feature extraction logic)
 *  Goal    : Detect left-recursive grammar rules that violate
 *            top-down (LL) parsing constraints.
 *
 *  Features computed per rule:
 *    - Recursion_Depth  : number of times the LHS non-terminal
 *                         appears on the RHS
 *    - Has_Left_Recursion: 1 if the first token on ANY alternative
 *                          of the RHS matches the LHS, else 0
 * ============================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_RULES     20
#define MAX_LEN      256
#define MAX_ALTS      10
#define MAX_TOKENS    20

/* ──────────────────────────────────────────────────────────── */
/*  Data structures                                             */
/* ──────────────────────────────────────────────────────────── */

typedef struct {
    char lhs[64];                   /* Non-terminal on left  */
    char rhs_raw[MAX_LEN];          /* Raw RHS string        */
    char alternatives[MAX_ALTS][MAX_LEN]; /* Each | alternative */
    int  alt_count;
    int  recursion_depth;           /* Feature               */
    int  has_left_recursion;        /* Flag  (0 or 1)        */
    char parser_ready[8];           /* "YES" or "NO"         */
} GrammarRule;

/* ──────────────────────────────────────────────────────────── */
/*  Utility: trim leading/trailing whitespace in-place          */
/* ──────────────────────────────────────────────────────────── */
void trim(char *s) {
    /* leading */
    int start = 0;
    while (s[start] && isspace((unsigned char)s[start])) start++;
    if (start) memmove(s, s + start, strlen(s) - start + 1);
    /* trailing */
    int end = (int)strlen(s) - 1;
    while (end >= 0 && isspace((unsigned char)s[end])) s[end--] = '\0';
}

/* ──────────────────────────────────────────────────────────── */
/*  Split RHS by '|' into alternatives array                    */
/* ──────────────────────────────────────────────────────────── */
void split_alternatives(GrammarRule *rule) {
    char tmp[MAX_LEN];
    strncpy(tmp, rule->rhs_raw, MAX_LEN - 1);
    tmp[MAX_LEN - 1] = '\0';

    rule->alt_count = 0;
    char *token = strtok(tmp, "|");
    while (token && rule->alt_count < MAX_ALTS) {
        strncpy(rule->alternatives[rule->alt_count], token, MAX_LEN - 1);
        trim(rule->alternatives[rule->alt_count]);
        rule->alt_count++;
        token = strtok(NULL, "|");
    }
}

/* ──────────────────────────────────────────────────────────── */
/*  Count how many times LHS appears anywhere in RHS            */
/*  (Recursion_Depth feature)                                   */
/* ──────────────────────────────────────────────────────────── */
int compute_recursion_depth(const GrammarRule *rule) {
    int count = 0;
    int lhs_len = (int)strlen(rule->lhs);
    const char *ptr = rule->rhs_raw;

    while ((ptr = strstr(ptr, rule->lhs)) != NULL) {
        /*
         * Make sure it is a whole-word match (not a substring of
         * a longer non-terminal, e.g. "E" must not match "E'")
         */
        int before_ok = (ptr == rule->rhs_raw) ||
                        !isalnum((unsigned char)*(ptr - 1));
        int after_ok  = !isalnum((unsigned char)*(ptr + lhs_len)) &&
                        *(ptr + lhs_len) != '\'';  /* avoid E' match */
        if (before_ok && after_ok) count++;
        ptr += lhs_len;
    }
    return count;
}

/* ──────────────────────────────────────────────────────────── */
/*  Check Has_Left_Recursion                                     */
/*  A rule  A -> A α | β  has left recursion when the           */
/*  FIRST token of any alternative equals LHS.                  */
/* ──────────────────────────────────────────────────────────── */
int check_left_recursion(const GrammarRule *rule) {
    int lhs_len = (int)strlen(rule->lhs);

    for (int i = 0; i < rule->alt_count; i++) {
        const char *alt = rule->alternatives[i];

        /* Skip epsilon (empty) alternatives */
        if (strcmp(alt, "eps") == 0 || strcmp(alt, "ε") == 0 ||
            strcmp(alt, "epsilon") == 0 || strlen(alt) == 0)
            continue;

        /* Does alternative START with the LHS non-terminal? */
        if (strncmp(alt, rule->lhs, lhs_len) == 0) {
            char next_char = alt[lhs_len];
            /* Ensure whole-word match (not E matching E') */
            if (!isalnum((unsigned char)next_char) && next_char != '\'') {
                return 1;   /* left recursion found */
            }
        }
    }
    return 0;
}

/* ──────────────────────────────────────────────────────────── */
/*  Pretty-print separator                                       */
/* ──────────────────────────────────────────────────────────── */
void print_separator(void) {
    printf("+----+----------+");
    for (int i = 0; i < 36; i++) putchar('-');
    printf("+------------------+------------------+---------------------+\n");
}

/* ──────────────────────────────────────────────────────────── */
/*  Main                                                         */
/* ──────────────────────────────────────────────────────────── */
int main(void) {

    /* ── Dataset: 10 grammar production rules ──────────────── */
    /*
     * Format:  LHS  →  RHS  (alternatives separated by |)
     * These represent common grammar patterns seen in real
     * compilers and textbooks.
     */
    GrammarRule rules[MAX_RULES];
    int n = 0;

    /* Rule 0: Direct left recursion – Arithmetic expressions */
    strcpy(rules[n].lhs, "E");
    strcpy(rules[n].rhs_raw, "E + T | T");
    n++;

    /* Rule 1: Direct left recursion – Term */
    strcpy(rules[n].lhs, "T");
    strcpy(rules[n].rhs_raw, "T * F | F");
    n++;

    /* Rule 2: No recursion – Factor */
    strcpy(rules[n].lhs, "F");
    strcpy(rules[n].rhs_raw, "( E ) | id");
    n++;

    /* Rule 3: Indirect left recursion – Statement list */
    strcpy(rules[n].lhs, "S");
    strcpy(rules[n].rhs_raw, "S ; stmt | stmt");
    n++;

    /* Rule 4: Right recursion (safe for LL) – Production list */
    strcpy(rules[n].lhs, "P");
    strcpy(rules[n].rhs_raw, "id -> RHS P | eps");
    n++;

    /* Rule 5: Direct left recursion – Addition chain */
    strcpy(rules[n].lhs, "A");
    strcpy(rules[n].rhs_raw, "A + B | B");
    n++;

    /* Rule 6: No recursion – Boolean literal */
    strcpy(rules[n].lhs, "BOOL");
    strcpy(rules[n].rhs_raw, "true | false");
    n++;

    /* Rule 7: Left recursion – Qualified name (a.b.c) */
    strcpy(rules[n].lhs, "QN");
    strcpy(rules[n].rhs_raw, "QN . id | id");
    n++;

    /* Rule 8: Right recursion – Argument list */
    strcpy(rules[n].lhs, "AL");
    strcpy(rules[n].rhs_raw, "expr , AL | expr");
    n++;

    /* Rule 9: Multiple recursion – Left-recursive expr with subtraction */
    strcpy(rules[n].lhs, "EX");
    strcpy(rules[n].rhs_raw, "EX + EX | EX - EX | num");
    n++;

    /* ── Compute features for each rule ────────────────────── */
    for (int i = 0; i < n; i++) {
        split_alternatives(&rules[i]);
        rules[i].recursion_depth    = compute_recursion_depth(&rules[i]);
        rules[i].has_left_recursion = check_left_recursion(&rules[i]);
        strcpy(rules[i].parser_ready,
               rules[i].has_left_recursion ? "NO" : "YES");
    }

    /* ── Print header ───────────────────────────────────────── */
    printf("\n");
    printf("==================================================================="
           "============================\n");
    printf("   GRAMMAR LEFT RECURSION RISK DETECTION  –  Feature Extraction Report\n");
    printf("==================================================================="
           "============================\n");
    printf("   Tool    : C (Data Processing + Flag Generation)\n");
    printf("   Feature : Recursion_Depth  |  Flag : Has_Left_Recursion\n");
    printf("   Goal    : Identify grammar rules that violate LL (top-down) "
           "parsing constraints\n");
    printf("==================================================================="
           "============================\n\n");

    /* ── Print table ────────────────────────────────────────── */
    print_separator();
    printf("| #  | LHS      | %-36s | Recursion_Depth  | Has_Left_Recursion  | Parser_Ready        |\n",
           "Production  (RHS)");
    print_separator();

    for (int i = 0; i < n; i++) {
        /* Truncate RHS display to 36 chars for table alignment */
        char rhs_display[40];
        snprintf(rhs_display, sizeof(rhs_display), "%s", rules[i].rhs_raw);
        if (strlen(rhs_display) > 35) {
            rhs_display[33] = '.';
            rhs_display[34] = '.';
            rhs_display[35] = '\0';
        }
        printf("| %-2d | %-8s | %-36s | %-16d | %-19s | %-19s |\n",
               i,
               rules[i].lhs,
               rhs_display,
               rules[i].recursion_depth,
               rules[i].has_left_recursion ? "1  (YES - RISK)" : "0  (No Risk)",
               rules[i].parser_ready);
    }
    print_separator();

    /* ── Summary statistics ─────────────────────────────────── */
    int total_lr = 0, total_safe = 0;
    for (int i = 0; i < n; i++) {
        if (rules[i].has_left_recursion) total_lr++;
        else total_safe++;
    }

    printf("\n");
    printf("┌─────────────────────────────────────────┐\n");
    printf("│           SUMMARY STATISTICS            │\n");
    printf("├─────────────────────────────────────────┤\n");
    printf("│  Total rules analysed   : %-14d │\n", n);
    printf("│  Left-recursive (risky) : %-14d │\n", total_lr);
    printf("│  Parser-ready (safe)    : %-14d │\n", total_safe);
    printf("│  Parser-readiness %%     : %-13.1f%% │\n",
           (double)total_safe / n * 100.0);
    printf("├─────────────────────────────────────────┤\n");
    printf("│  VERDICT : %-30s│\n",
           total_lr > 0 ? "Grammar NEEDS elimination step!" :
                          "Grammar is LL-parser ready!");
    printf("└─────────────────────────────────────────┘\n");

    /* ── Explanation of flagged rules ───────────────────────── */
    printf("\n  LEFT-RECURSIVE RULES DETECTED (Require elimination before LL parsing):\n");
    printf("  ─────────────────────────────────────────────────────────────────────\n");
    for (int i = 0; i < n; i++) {
        if (rules[i].has_left_recursion) {
            printf("  Rule %d : %s -> %s\n", i, rules[i].lhs, rules[i].rhs_raw);
            printf("           Recursion_Depth = %d  |  Apply left-recursion "
                   "elimination →  %s -> %s' %s'\n",
                   rules[i].recursion_depth,
                   rules[i].lhs, rules[i].lhs, rules[i].lhs);
        }
    }

    printf("\n  HOW TO ELIMINATE (standard technique):\n");
    printf("  For each   A -> A α | β\n");
    printf("  Replace with:\n");
    printf("             A  -> β A'\n");
    printf("             A' -> α A' | ε\n\n");

    printf("  This removes left recursion while preserving the language generated.\n\n");

    return 0;
}
