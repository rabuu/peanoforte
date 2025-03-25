#include "print.h"
#include "parser.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

/* forward declarations */
Expr *find_marked_expr(Expr *expr);

void assert_no_marked_exprs(ExprList *list) {
	if (!list) return;
	if (find_marked_expr(list->head)) {
		printf("** ERROR ** Only one subexpression can be marked: ");
		print_expr(list->head);
		exit(1);
	}
	return assert_no_marked_exprs(list->tail);
}

Expr *find_marked_expr_in_list(ExprList *list) {
	if (!list) return nullptr;
	if (find_marked_expr(list->head)) {
		assert_no_marked_exprs(list->tail);
		return list->head;
	}
	return find_marked_expr_in_list(list->tail);
}

Expr *find_marked_expr(Expr *expr) {
	if (!expr) return nullptr;

	Expr *found = nullptr;
	if (expr->marked) found = expr;

	switch (expr->tag) {
		case EXPR_ZERO:
		case EXPR_VAR:
			break;
		case EXPR_SEXP:
			if (found)
				assert_no_marked_exprs(expr->sexp);
			else
				found = find_marked_expr_in_list(expr->sexp);
			break;
	}

	return found;
}

void check_axiom(Axiom *axiom) {
	/* check that exprs are unmarked */
	if (find_marked_expr(axiom->lhs)) {
		printf("WARN: LHS of axiom %s contains mark: ", axiom->name);
		print_expr(axiom->lhs);
	}
	if (find_marked_expr(axiom->rhs)) {
		printf("WARN: RHS of axiom %s contains mark: ", axiom->name);
		print_expr(axiom->rhs);
	}
}

void verify_program(Program *ast) {
	if (!ast) return;

	switch (ast->toplevel.tag) {
		case TOPLEVEL_AXIOM:
			check_axiom(&ast->toplevel.axiom);
			break;
		case TOPLEVEL_THEOREM:
		case TOPLEVEL_EXAMPLE:
			break;
	}

	verify_program(ast->rest);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("ERROR: Please provide a filename.\n");
		return 1;
	}

	printf("*** PARSING *** filename: `%s`\n", argv[1]);
	Program *program; 
	int parse_error = parse(argv[1], &program);
	if (!parse_error) printf("SUCCESS\n");

	if (true) {
		printf("\n*** DEBUG PRINT ***\n-------------------\n");
		print_program(program);
	}
	if (parse_error) return parse_error;

	printf("\n*** VERIFY ***\n----------------\n");
	verify_program(program);

	return parse_error;
}
