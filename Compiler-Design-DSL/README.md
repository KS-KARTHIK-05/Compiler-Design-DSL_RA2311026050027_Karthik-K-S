# Simple Imperative DSL Compiler
### Compiler Design Assignment — `Compiler-Design-DSL_RegNo_Name`

---

## Project Description

A fully functional compiler for a **Simple Imperative Domain-Specific Language (DSL)** built from scratch using industry-standard compiler tools. The DSL supports variables, arithmetic, conditionals, loops, and I/O — equivalent in expressiveness to a simplified C subset.

This project implements all five canonical compiler phases:

```
Source Code (.dsl)
       │
       ▼
  ┌─────────────┐
  │  Lexer      │  flex (lexer.l)    → tokens
  └──────┬──────┘
         ▼
  ┌─────────────┐
  │  Parser     │  bison (parser.y)  → parse tree / AST
  └──────┬──────┘
         ▼
  ┌─────────────┐
  │  AST Build  │  C (ast.h/ast.c)   → typed tree nodes
  └──────┬──────┘
         ▼
  ┌─────────────┐
  │  Semantic   │  C (semantic.c)    → type/scope checks
  └──────┬──────┘
         ▼
  ┌─────────────┐
  │  ICG (TAC)  │  C (codegen.c)     → Three-Address Code
  └─────────────┘
```

---

## DSL Language Features

```c
// Variable declarations
int x;
bool flag;

// Assignment
x = 10 + 3 * 2;

// If / Else
if (x > 5) {
    print(x);
} else {
    print(0);
}

// While loop
while (x > 0) {
    x = x - 1;
}

// Print (int, bool, or string)
print("Hello from DSL!");
print(x);
```

**Supported operators:**
- Arithmetic: `+  -  *  /  %`
- Relational: `==  !=  <  >  <=  >=`
- Logical:    `&&  ||  !`

---

## Tools Used

| Tool | Version | Purpose |
|------|---------|---------|
| GCC  | 11+     | C compiler |
| Flex | 2.6.4   | Lexer generator |
| Bison| 3.8+    | Parser generator (LALR) |
| Make | 4.3+    | Build automation |

---

## Folder Structure

```
Compiler-Design-DSL/
├── src/
│   ├── lexer.l          # Flex lexical analyser
│   ├── parser.y         # Bison grammar + main driver
│   ├── ast.h / ast.c    # AST node definitions & utilities
│   ├── symtab.h         # Symbol table (hash table)
│   ├── semantic.h / .c  # Semantic analysis
│   └── codegen.h / .c   # Three-Address Code generation
├── test/
│   ├── test1.dsl        # Comprehensive test (if/while/bool/logic)
│   └── test2.dsl        # Simple hello-world arithmetic test
├── output/
│   ├── test1_output.txt # Compiler output for test1
│   ├── test2_output.txt # Compiler output for test2
│   └── tac_output.txt   # TAC intermediate code
├── docs/
│   └── report.pdf       # Architecture & module explanation
├── Makefile
└── README.md
```

---

## Steps to Run (Ubuntu 22 / Linux)

### 1. Install dependencies
```bash
sudo apt update
sudo apt install flex bison gcc make -y
```

### 2. Clone the repository
```bash
git clone https://github.com/<your-username>/Compiler-Design-DSL_RegNo_Name.git
cd Compiler-Design-DSL_RegNo_Name
```

### 3. Build the compiler
```bash
make
```

### 4. Run on a test file
```bash
./compiler test/test1.dsl
./compiler test/test2.dsl
```

### 5. Run all tests (saves output files)
```bash
make test
```

### 6. Clean build artifacts
```bash
make clean
```

---

## Sample Input (test2.dsl)

```c
int a;
int b;
int sum;

a = 5;
b = 7;
sum = a + b;

print("Sum is:");
print(sum);

if (sum > 10) {
    print("Greater than 10");
}
```

## Sample Output

```
============================================================
  Simple Imperative DSL Compiler
  Modules: Lex → Parse → AST → Semantic → ICG (TAC)
============================================================

[LEXER]    Tokenizing 'test/test2.dsl'...
[PARSER]   Parsing source...
[PARSER]   Parse successful.

[AST]      Abstract Syntax Tree:
──────────────────────────────────────────────────────────
[PROGRAM]
  [DECL_INT: a]
  [DECL_INT: b]
  [DECL_INT: sum]
  [ASSIGN]
    [ID: a]
    [NUM: 5]
  ...

[SEMANTIC] Analysis passed with 0 errors.

[CODEGEN]  Three-Address Code (TAC):
  │ 7    │ +          │ a, b         │ t2               │
  │ 8    │ =          │ t2           │ sum              │
  ...

[COMPILER] Compilation successful!
============================================================
```

---

## Architecture Overview

### Lexer (`lexer.l`)
Converts raw source text into a stream of tokens using regular expressions. Handles keywords, identifiers, literals, operators, and comments. Reports lexical errors with line numbers.

### Parser (`parser.y`)
LALR(1) grammar defined in Bison. Builds the AST by associating semantic actions with each grammar production. Handles operator precedence and associativity.

### AST (`ast.h`, `ast.c`)
24 node types covering all language constructs. Each node stores its type tag, payload (integer value or string), up to 3 child pointers, and a sibling pointer for statement lists.

### Symbol Table (`symtab.h`)
Hash-table implementation (64 buckets, djb2 hash). Stores name, type, declaration line, and usage flag per variable. Detects duplicates in O(1) average time.

### Semantic Analysis (`semantic.c`)
Tree-walk pass that checks: undeclared variables, duplicate declarations, type mismatches in assignments, invalid condition types, and constant-value division by zero.

### Code Generator (`codegen.c`)
Recursive tree-walk emitting Three-Address Code (TAC). Uses fresh temporaries (t0, t1, …) and labels (L0, L1, …). Generates correct GOTO/IF_FALSE/LABEL sequences for control flow.

---

## Team Details

| Name | Reg No | Role |
|------|--------|------|
| *(Your Name)* | *(Your Reg No)* | All modules |

**Institution:** SRM Institute of Science and Technology, Tiruchirappalli  
**Department:** Artificial Intelligence & Machine Learning  
**Subject:** Compiler Design  
**Academic Year:** 2025–2026
