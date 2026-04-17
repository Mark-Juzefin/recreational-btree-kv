CC      ?= clang
CSTD    ?= -std=c11
WARN    := -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion \
           -Wnull-dereference -Wdouble-promotion -Wunused -Werror=return-type
OPT     ?= -O2 -g
CFLAGS  := $(CSTD) $(WARN) $(OPT) -Isrc
LDFLAGS :=

SRC := src/pager.c src/table.c src/btree.c src/db.c
OBJ := $(SRC:.c=.o)

.PHONY: all clean test asan release help

all: kvdb

kvdb: $(OBJ) src/main.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test: tests/test_db
	./tests/test_db

tests/test_db: $(OBJ) tests/test_db.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Build everything with AddressSanitizer + UndefinedBehaviorSanitizer.
# Usage: make clean && make asan && ./tests/test_db
asan: CFLAGS += -O1 -fno-omit-frame-pointer -fsanitize=address,undefined
asan: LDFLAGS += -fsanitize=address,undefined
asan: clean kvdb tests/test_db

release: CFLAGS := $(CSTD) $(WARN) -O3 -DNDEBUG -Isrc
release: clean kvdb

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) src/main.o tests/*.o kvdb tests/test_db
	rm -f *.idx *.data tests/*.idx tests/*.data

help:
	@echo "Targets:"
	@echo "  make          - build kvdb (default)"
	@echo "  make test     - build and run tests"
	@echo "  make asan     - build with ASan+UBSan"
	@echo "  make release  - optimized build"
	@echo "  make clean    - remove build artifacts"
