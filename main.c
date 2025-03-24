#include "print.h"
#include "parser.h"
#include "ast.h"
#include <stdio.h>

void check_axiom(Axiom *axiom) {
	Expr *marked = expr_find_marked(axiom->lhs);
	if (marked) {
		print_expr(marked);
	} else {
		printf("NO MARK\n");
	}
}

void check(Program *ast) {
	if (!ast) return;

	switch (ast->toplevel.tag) {
		case TOPLEVEL_AXIOM:
			check_axiom(&ast->toplevel.axiom);
			break;
		case TOPLEVEL_THEOREM:
		case TOPLEVEL_EXAMPLE:
			break;
	}

	check(ast->rest);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("ERROR: Please provide a filename.\n");
		return 1;
	}

	printf("Try to parse %s...\n", argv[1]);
	Program *ast; 
	int parse_success = parse(argv[1], &ast);

	printf("\n*** DEBUG PRINT ***\n-------------------\n");
	print_program(ast);

	printf("\n*** CHECK ***\n--------------\n");
	check(ast);

	return parse_success;
}
