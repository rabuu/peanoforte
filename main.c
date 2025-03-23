#include "print.h"
#include "parser.h"
#include "ast.h"
#include <stdio.h>

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("ERROR: Please provide a filename.\n");
		return 1;
	}

	printf("Try to parse %s...\n", argv[1]);
	PF_Program *ast; 
	int parse_success = PF_parse(argv[1], &ast);

	PF_print_program(ast);

	return parse_success;
}
