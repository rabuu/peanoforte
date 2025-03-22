CC = cc
# CFLAGS = -Wextra -Wall -std=c23

peanoforte: lexer.c parser.c
	$(CC) $(CFLAGS) $^ -o $@

lexer.h lexer.c: lexer.l
	flex --header-file=lexer.h -o lexer.c lexer.l

parser.h parser.c: parser.y
	bison -d -o parser.c parser.y

.PHONY: clean

clean:
	rm -rf *.o lexer.h lexer.c parser.h parser.c peanoforte
