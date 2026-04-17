CC     := clang
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g -O0

.PHONY: run clean

main: main.c
	$(CC) $(CFLAGS) -o $@ $<

run: main
	./main

clean:
	rm -f main
