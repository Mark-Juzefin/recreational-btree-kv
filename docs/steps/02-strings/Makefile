SHELL  := /bin/bash
CC     := clang
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g -O0

ARGS ?=

.PHONY: run run-tests clean

echoer: echoer.c
	$(CC) $(CFLAGS) -o $@ $<

run: echoer
	./echoer $(ARGS)

# Quick scripted checks. Each line: run, compare output, report.
run-tests: echoer
	@echo '--- test 1: plain ---'
	@./echoer hello world | diff - <(printf 'hello (5)\nworld (5)\n') && echo OK
	@echo '--- test 2: upper ---'
	@./echoer --upper hi there | diff - <(printf 'HI (2)\nTHERE (5)\n') && echo OK
	@echo '--- test 3: upper flag anywhere ---'
	@./echoer hi --upper there | diff - <(printf 'HI (2)\nTHERE (5)\n') && echo OK
	@echo '--- test 4: no args ---'
	@./echoer | diff - /dev/null && echo OK

clean:
	rm -f echoer
