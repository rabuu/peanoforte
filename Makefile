CC = cc
CFLAGS = -Wextra -Wall -std=c23

peanoforte: main.c lexer.c parser.c ast.c print.c
	$(CC) $(CFLAGS) $^ -o $@

lexer.h lexer.c: lexer.l
	flex --header-file=lexer.h -o lexer.c lexer.l

parser.h parser.c: parser.y
	bison --header -o parser.c parser.y

.PHONY: clean fmt bison-verbose

clean:
	rm -rf *.o lexer.h lexer.c parser.h parser.c peanoforte

bison-verbose:
	bison --verbose --header -o parser.c parser.y

fmt:
	clang-format -i -- *.c *.h
