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
	if (!axiom) {
		printf("ERROR: Axiom is NULL\n");
		exit(1);
	}

	printf("\nAXIOM %s ", axiom->name);
	if (axiom->params) printf("<");
	_print_ident_list(axiom->params);
	if (axiom->params) printf("> ");
	_PF_print_expr(axiom->lhs);
	printf(" = ");
	_PF_print_expr(axiom->rhs);
	printf("\n");
}

void print_axioms(PF_AxiomList *axioms) {
	if (!axioms) return;
	print_axiom(axioms->head);
	print_axioms(axioms->tail);
}

void print_theorem(PF_Theorem *theorem) {
	if (!theorem) {
		printf("ERROR: Theorem is NULL\n");
		exit(1);
	}

	printf("\nTHEOREM %s ", theorem->name);
	if (theorem->params) printf("<");
	_print_ident_list(theorem->params);
	if (theorem->params) printf("> ");
	_PF_print_expr(theorem->lhs);
	printf(" = ");
	_PF_print_expr(theorem->rhs);
	printf("\n");
}

void print_theorems(PF_TheoremList *theorems) {
	if (!theorems) return;
	print_theorem(theorems->head);
	print_theorems(theorems->tail);
}

void PF_print_program(PF_Program *prog) {
	if (!prog) {
		printf("ERROR: Program is NULL\n");
		exit(1);
	}

	print_axioms(prog->axioms);
	print_theorems(prog->theorems);
}
