%{
/*
 * parser.y  –  YACC/Bison grammar for Simple Imperative DSL
 *
 * Grammar summary:
 *   program   → stmt_list
 *   stmt_list → stmt*
 *   stmt      → decl | assign | if_stmt | while_stmt | print_stmt
 *   decl      → (INT | BOOL) IDENTIFIER ;
 *   assign    → IDENTIFIER = expr ;
 *   if_stmt   → if ( expr ) { stmt_list } [else { stmt_list }]
 *   while_stmt→ while ( expr ) { stmt_list }
 *   print_stmt→ print ( expr ) ;
 *   expr      → arithmetic | relational | logical
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"
#include "semantic.h"
#include "codegen.h"

extern int  yylex(void);
extern int  line_num;
extern FILE *yyin;

void yyerror(const char *msg);

/* Global AST root */
ASTNode *ast_root = NULL;
%}

/* ── Value type union ──────────────────────────────────────── */
%union {
    int       ival;
    char     *sval;
    ASTNode  *node;
}

/* ── Token declarations ────────────────────────────────────── */
%token <ival> NUMBER TRUE_LIT FALSE_LIT
%token <sval> IDENTIFIER STRING_LIT

%token IF ELSE WHILE PRINT
%token INT BOOL
%token ASSIGN
%token PLUS MINUS MUL DIV MOD
%token EQ NEQ LT GT LEQ GEQ
%token AND OR NOT
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA

/* ── Operator precedence (low → high) ──────────────────────── */
%left  OR
%left  AND
%right NOT
%left  EQ NEQ
%left  LT GT LEQ GEQ
%left  PLUS MINUS
%left  MUL DIV MOD
%right UMINUS

/* ── Non-terminal types ────────────────────────────────────── */
%type <node> program stmt_list stmt
%type <node> decl_stmt assign_stmt
%type <node> if_stmt while_stmt print_stmt
%type <node> expr

%%

/* ── Program ─────────────────────────────────────────────── */
program
    : stmt_list
        {
            ASTNode *n = new_node(NODE_PROGRAM);
            n->left   = $1;
            ast_root  = n;
            $$ = n;
        }
    ;

/* ── Statement list ──────────────────────────────────────── */
stmt_list
    : /* empty */
        { $$ = NULL; }
    | stmt_list stmt
        {
            /* Append $2 to the end of the list in $1 */
            if ($1 == NULL) {
                $$ = $2;
            } else {
                /* Find tail */
                ASTNode *tail = $1;
                while (tail->next) tail = tail->next;
                tail->next = $2;
                $$ = $1;
            }
        }
    ;

/* ── Single statement ────────────────────────────────────── */
stmt
    : decl_stmt    { $$ = $1; }
    | assign_stmt  { $$ = $1; }
    | if_stmt      { $$ = $1; }
    | while_stmt   { $$ = $1; }
    | print_stmt   { $$ = $1; }
    ;

/* ── Declaration ─────────────────────────────────────────── */
decl_stmt
    : INT IDENTIFIER SEMICOLON
        {
            ASTNode *n = new_node(NODE_DECL_INT);
            n->sval   = $2;
            n->line   = line_num;
            $$ = n;
        }
    | BOOL IDENTIFIER SEMICOLON
        {
            ASTNode *n = new_node(NODE_DECL_BOOL);
            n->sval   = $2;
            n->line   = line_num;
            $$ = n;
        }
    ;

/* ── Assignment ──────────────────────────────────────────── */
assign_stmt
    : IDENTIFIER ASSIGN expr SEMICOLON
        {
            ASTNode *lhs = make_id($1);
            free($1);
            ASTNode *n   = make_binop(NODE_ASSIGN, lhs, $3);
            n->line = line_num;
            $$ = n;
        }
    ;

/* ── If statement ────────────────────────────────────────── */
if_stmt
    : IF LPAREN expr RPAREN LBRACE stmt_list RBRACE
        {
            ASTNode *n  = new_node(NODE_IF);
            n->left     = $3;       /* condition  */
            n->right    = $6;       /* then-body  */
            n->extra    = NULL;     /* no else    */
            n->line     = line_num;
            $$ = n;
        }
    | IF LPAREN expr RPAREN LBRACE stmt_list RBRACE
      ELSE LBRACE stmt_list RBRACE
        {
            ASTNode *n  = new_node(NODE_IF);
            n->left     = $3;       /* condition  */
            n->right    = $6;       /* then-body  */
            n->extra    = $10;      /* else-body  */
            n->line     = line_num;
            $$ = n;
        }
    ;

/* ── While statement ─────────────────────────────────────── */
while_stmt
    : WHILE LPAREN expr RPAREN LBRACE stmt_list RBRACE
        {
            ASTNode *n  = new_node(NODE_WHILE);
            n->left     = $3;       /* condition */
            n->right    = $6;       /* body      */
            n->line     = line_num;
            $$ = n;
        }
    ;

/* ── Print statement ─────────────────────────────────────── */
print_stmt
    : PRINT LPAREN expr RPAREN SEMICOLON
        {
            ASTNode *n = new_node(NODE_PRINT);
            n->left    = $3;
            n->line    = line_num;
            $$ = n;
        }
    ;

/* ── Expressions ─────────────────────────────────────────── */
expr
    : expr PLUS  expr   { $$ = make_binop(NODE_ADD, $1, $3); }
    | expr MINUS expr   { $$ = make_binop(NODE_SUB, $1, $3); }
    | expr MUL   expr   { $$ = make_binop(NODE_MUL, $1, $3); }
    | expr DIV   expr   { $$ = make_binop(NODE_DIV, $1, $3); }
    | expr MOD   expr   { $$ = make_binop(NODE_MOD, $1, $3); }

    | expr EQ    expr   { $$ = make_binop(NODE_EQ,  $1, $3); }
    | expr NEQ   expr   { $$ = make_binop(NODE_NEQ, $1, $3); }
    | expr LT    expr   { $$ = make_binop(NODE_LT,  $1, $3); }
    | expr GT    expr   { $$ = make_binop(NODE_GT,  $1, $3); }
    | expr LEQ   expr   { $$ = make_binop(NODE_LEQ, $1, $3); }
    | expr GEQ   expr   { $$ = make_binop(NODE_GEQ, $1, $3); }

    | expr AND   expr   { $$ = make_binop(NODE_AND, $1, $3); }
    | expr OR    expr   { $$ = make_binop(NODE_OR,  $1, $3); }
    | NOT  expr         { $$ = make_unop (NODE_NOT, $2);     }

    | MINUS expr %prec UMINUS
        { $$ = make_unop(NODE_NEG, $2); }

    | LPAREN expr RPAREN { $$ = $2; }

    | NUMBER     { $$ = make_num($1);  }
    | TRUE_LIT   { $$ = make_bool(1);  }
    | FALSE_LIT  { $$ = make_bool(0);  }
    | STRING_LIT { $$ = make_str($1); free($1); }
    | IDENTIFIER { $$ = make_id($1);  free($1); }
    ;

%%

/* ── Error handler ───────────────────────────────────────── */
void yyerror(const char *msg) {
    fprintf(stderr, "[PARSE ERROR] Line %d: %s\n", line_num, msg);
}

/* ── Main driver ─────────────────────────────────────────── */
int main(int argc, char *argv[]) {

    printf("============================================================\n");
    printf("  Simple Imperative DSL Compiler\n");
    printf("  Modules: Lex → Parse → AST → Semantic → ICG (TAC)\n");
    printf("============================================================\n\n");

    /* Open input file */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file.dsl>\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("fopen");
        return 1;
    }
    printf("[LEXER]    Tokenizing '%s'...\n", argv[1]);

    /* ── Phase 1 & 2: Lex + Parse ───────────────────────── */
    printf("[PARSER]   Parsing source...\n");
    if (yyparse() != 0) {
        fprintf(stderr, "[COMPILER] Parse failed. Aborting.\n");
        fclose(yyin);
        return 1;
    }
    fclose(yyin);
    printf("[PARSER]   Parse successful.\n\n");

    /* ── Phase 3: AST Dump ──────────────────────────────── */
    printf("[AST]      Abstract Syntax Tree:\n");
    printf("─────────────────────────────────────────────────────────\n");
    ast_print(ast_root, 0);
    printf("─────────────────────────────────────────────────────────\n\n");

    /* ── Phase 4: Semantic Analysis ─────────────────────── */
    printf("[SEMANTIC] Running semantic analysis...\n");
    SymTab *st = symtab_new();
    int sem_errors = semantic_check(ast_root, st);

    symtab_print(st);

    if (sem_errors > 0) {
        fprintf(stderr,
            "[COMPILER] Compilation aborted due to semantic errors.\n");
        symtab_free(st);
        ast_free(ast_root);
        return 1;
    }

    /* ── Phase 5: Intermediate Code Generation ───────────── */
    printf("\n[CODEGEN]  Generating Three-Address Code (TAC)...\n");
    TACList *tac = taclist_new();
    codegen(ast_root, tac, st);
    taclist_print(tac, stdout);

    /* Write TAC to output file too */
    FILE *tacf = fopen("output/tac_output.txt", "w");
    if (tacf) {
        fprintf(tacf, "TAC Output for: %s\n", argv[1]);
        taclist_print(tac, tacf);
        fclose(tacf);
        printf("[CODEGEN]  TAC written to output/tac_output.txt\n");
    }

    printf("\n[COMPILER] Compilation successful!\n");
    printf("============================================================\n\n");

    /* ── Cleanup ─────────────────────────────────────────── */
    taclist_free(tac);
    symtab_free(st);
    ast_free(ast_root);
    return 0;
}
