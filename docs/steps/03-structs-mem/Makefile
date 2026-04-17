SHELL  := /bin/bash
CC     := clang
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g -O0

SRC := intvec.c intvec_test.c

.PHONY: run asan clean

run: test
	./test

test: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
asan: clean test
	./test

clean:
	rm -f test
