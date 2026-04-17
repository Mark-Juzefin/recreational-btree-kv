CC     := clang
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g -O0

.PHONY: run clean

hello: hello.c
	$(CC) $(CFLAGS) -o $@ $<

run: hello
	./hello

clean:
	rm -f hello
