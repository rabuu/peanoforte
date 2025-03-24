#include "print.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

/* forward declarations */
void _PF_print_expr(PF_Expr *expr);

void _print_ident_list(PF_IdentList *idents) {
	if (!idents) return;
	printf("%s", idents->ident);
	if (idents->tail) printf(", ");
	_print_ident_list(idents->tail);
}

void _print_sexp_inner(PF_ExprList *sexp) {
	if (!sexp) return;
	_PF_print_expr(sexp->head);
	if (sexp->tail) printf(" ");
	_print_sexp_inner(sexp->tail);
}

void _print_sexp(PF_ExprList *sexp) {
	printf("(");
	_print_sexp_inner(sexp);
	printf(")");
}

void _PF_print_expr(PF_Expr *expr) {
	if (!expr) {
		printf("ERROR: Expr is NULL\n");
		exit(1);
	}

	switch (expr->kind) {
		case PF_EXPR_ZERO: printf("0"); break;
		case PF_EXPR_VAR: printf("%s", expr->var); break;
		case PF_EXPR_SEXP: _print_sexp(expr->sexp); break;
	}
}

void PF_print_expr(PF_Expr *expr) {
	_PF_print_expr(expr);
	printf("\n");
}

void print_axiom(PF_Axiom *axiom) {
	printf("AXIOM %s ", axiom->name);
	if (axiom->params) printf("<");
	_print_ident_list(axiom->params);
	if (axiom->params) printf("> ");
	_PF_print_expr(axiom->lhs);
	printf(" = ");
	_PF_print_expr(axiom->rhs);
	printf("\n");
}

void print_theorem(PF_Theorem *theorem) {
	printf("THEOREM %s ", theorem->name);
	if (theorem->params) printf("<");
	_print_ident_list(theorem->params);
	if (theorem->params) printf("> ");
	_PF_print_expr(theorem->lhs);
	printf(" = ");
	_PF_print_expr(theorem->rhs);
	printf("\n");
}

void print_toplevel(PF_TopLevel *toplevel) {
	if (!toplevel) {
		printf("ERROR: Toplevel is NULL\n");
		exit(1);
	}

	switch (toplevel->kind) {
		case PF_TOPLEVEL_AXIOM: print_axiom(&toplevel->axiom); break;
		case PF_TOPLEVEL_THEOREM: print_theorem(&toplevel->theorem); break;
	}
}

void PF_print_program(PF_Program *program) {
	if (!program) return;
	print_toplevel(&program->toplevel);
	if (program->rest) {
		printf("\n");
		PF_print_program(program->rest);
	}
}
