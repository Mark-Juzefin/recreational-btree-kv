SHELL  := /bin/bash
CC     := clang
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror=return-type -g -O0

SRC := bytefile.c bytefile_test.c

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
	rm -f test test-asan /tmp/bytefile_test_*.bin
