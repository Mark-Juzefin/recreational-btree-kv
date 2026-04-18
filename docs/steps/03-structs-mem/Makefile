SHELL  := /bin/bash
CC     := clang
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g -O0

SRC := intvec.c intvec_test.c

.PHONY: run asan leaks check clean

run: test
	./test

test: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

test-asan: $(SRC)
	$(CC) $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -o $@ $(SRC)

asan: test-asan
	./test-asan

leaks: test
	MallocStackLogging=1 leaks --atExit -- ./test

check: asan leaks

clean:
	rm -f test test-asan
