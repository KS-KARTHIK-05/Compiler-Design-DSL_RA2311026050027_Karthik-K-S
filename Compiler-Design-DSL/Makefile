# ============================================================
#  Makefile – Simple Imperative DSL Compiler
#  Tools: flex (lexer) + bison (parser) + gcc
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -g -I./src
FLEX    = flex
BISON   = bison

SRC     = src
OUT     = output

# ── Default target ───────────────────────────────────────────
all: $(OUT) compiler

# ── Create output directory ──────────────────────────────────
$(OUT):
	mkdir -p $(OUT)

# ── Step 1: Run Bison (generates parser.tab.c + parser.tab.h) ──
$(SRC)/parser.tab.c $(SRC)/parser.tab.h: $(SRC)/parser.y
	$(BISON) -d -o $(SRC)/parser.tab.c $(SRC)/parser.y

# ── Step 2: Run Flex (generates lex.yy.c) ─────────────────────
$(SRC)/lex.yy.c: $(SRC)/lexer.l $(SRC)/parser.tab.h
	$(FLEX) -o $(SRC)/lex.yy.c $(SRC)/lexer.l

# ── Step 3: Compile all C sources ─────────────────────────────
compiler: $(SRC)/parser.tab.c $(SRC)/lex.yy.c \
          $(SRC)/ast.c $(SRC)/semantic.c $(SRC)/codegen.c
	$(CC) $(CFLAGS) \
	    $(SRC)/parser.tab.c \
	    $(SRC)/lex.yy.c     \
	    $(SRC)/ast.c        \
	    $(SRC)/semantic.c   \
	    $(SRC)/codegen.c    \
	    -o compiler

# ── Run test 1 ────────────────────────────────────────────────
test1: compiler
	@echo ""
	@echo "====== Running test1.dsl ======"
	./compiler test/test1.dsl 2>&1 | tee $(OUT)/test1_output.txt

# ── Run test 2 ────────────────────────────────────────────────
test2: compiler
	@echo ""
	@echo "====== Running test2.dsl ======"
	./compiler test/test2.dsl 2>&1 | tee $(OUT)/test2_output.txt

# ── Run all tests ─────────────────────────────────────────────
test: test1 test2
	@echo ""
	@echo "All tests complete. Outputs saved in $(OUT)/"

# ── Clean ─────────────────────────────────────────────────────
clean:
	rm -f $(SRC)/parser.tab.c $(SRC)/parser.tab.h
	rm -f $(SRC)/lex.yy.c
	rm -f compiler
	rm -f $(OUT)/*.txt

.PHONY: all test test1 test2 clean
