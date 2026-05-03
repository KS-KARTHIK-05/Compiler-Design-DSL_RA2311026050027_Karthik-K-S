/*
 * symtab.h  –  Symbol Table for Semantic Analysis
 *
 * A simple hash-table based symbol table that stores:
 *   name, type (int / bool), declared (line), used
 *
 * Supports scoped lookup (for future extension).
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Value types ─────────────────────────────────────────── */
typedef enum {
    TYPE_UNKNOWN = 0,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_VOID
} VarType;

static const char *type_name(VarType t) {
    switch (t) {
        case TYPE_INT:    return "int";
        case TYPE_BOOL:   return "bool";
        case TYPE_STRING: return "string";
        case TYPE_VOID:   return "void";
        default:          return "unknown";
    }
}

/* ── Symbol entry ────────────────────────────────────────── */
typedef struct Symbol {
    char   *name;
    VarType type;
    int     decl_line;
    int     used;
    struct Symbol *next;   /* chaining for collisions */
} Symbol;

/* ── Symbol table ────────────────────────────────────────── */
#define SYMTAB_SIZE 64

typedef struct {
    Symbol *buckets[SYMTAB_SIZE];
    int     count;
} SymTab;

/* ── Hash function ───────────────────────────────────────── */
static unsigned int sym_hash(const char *name) {
    unsigned int h = 5381;
    while (*name) h = ((h << 5) + h) ^ (unsigned char)*name++;
    return h % SYMTAB_SIZE;
}

/* ── Create a new symbol table ───────────────────────────── */
static inline SymTab *symtab_new(void) {
    SymTab *st = (SymTab *)calloc(1, sizeof(SymTab));
    if (!st) { perror("calloc"); exit(1); }
    return st;
}

/* ── Insert (returns 0 on success, -1 if already declared) ─ */
static inline int symtab_insert(SymTab *st, const char *name,
                                VarType type, int line) {
    unsigned int h = sym_hash(name);
    Symbol *s = st->buckets[h];
    while (s) {
        if (strcmp(s->name, name) == 0) return -1; /* duplicate */
        s = s->next;
    }
    Symbol *entry = (Symbol *)calloc(1, sizeof(Symbol));
    entry->name      = strdup(name);
    entry->type      = type;
    entry->decl_line = line;
    entry->used      = 0;
    entry->next      = st->buckets[h];
    st->buckets[h]   = entry;
    st->count++;
    return 0;
}

/* ── Lookup (returns Symbol* or NULL) ────────────────────── */
static inline Symbol *symtab_lookup(SymTab *st, const char *name) {
    unsigned int h = sym_hash(name);
    Symbol *s = st->buckets[h];
    while (s) {
        if (strcmp(s->name, name) == 0) return s;
        s = s->next;
    }
    return NULL;
}

/* ── Mark a symbol as used ───────────────────────────────── */
static inline void symtab_mark_used(SymTab *st, const char *name) {
    Symbol *s = symtab_lookup(st, name);
    if (s) s->used = 1;
}

/* ── Print entire symbol table ───────────────────────────── */
static inline void symtab_print(SymTab *st) {
    printf("\n  ┌────────────────────────────────────┐\n");
    printf("  │        SYMBOL TABLE DUMP           │\n");
    printf("  ├──────────────┬────────┬────────────┤\n");
    printf("  │ %-12s │ %-6s │ %-10s │\n", "Name", "Type", "Decl Line");
    printf("  ├──────────────┼────────┼────────────┤\n");
    for (int i = 0; i < SYMTAB_SIZE; i++) {
        Symbol *s = st->buckets[i];
        while (s) {
            printf("  │ %-12s │ %-6s │ %-10d │\n",
                   s->name, type_name(s->type), s->decl_line);
            s = s->next;
        }
    }
    printf("  └──────────────┴────────┴────────────┘\n");
}

/* ── Free symbol table ───────────────────────────────────── */
static inline void symtab_free(SymTab *st) {
    for (int i = 0; i < SYMTAB_SIZE; i++) {
        Symbol *s = st->buckets[i];
        while (s) {
            Symbol *tmp = s->next;
            free(s->name);
            free(s);
            s = tmp;
        }
    }
    free(st);
}

#endif /* SYMTAB_H */
