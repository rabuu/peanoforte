#include "print.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

/* forward declarations */
void _PF_print_expr(PF_Expr *expr);
void print_node_transform(PF_ProofNodeTransform *transform);

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

void print_node_expr(PF_ProofNodeExpr *expr) {
	if (!expr) return;
	printf("EXPR: ");
	PF_print_expr(expr->expr);
	if (expr->transform) print_node_transform(expr->transform);
}

void print_node_transform(PF_ProofNodeTransform *transform) {
	if (!transform) return;
	printf("TRANSFORM");
	if (transform->reversed) printf(" (REVERSED)");
	printf(": %s\n", transform->name);
	if (transform->expr) print_node_expr(transform->expr);
}

void print_proof_direct(PF_Expr *start, PF_ProofNodeTransform *initial_transform) {
	printf("START: ");
	if (start) _PF_print_expr(start); else printf("IMPLIED");
	printf("\n");
	print_node_transform(initial_transform);
}

void print_proof(PF_Proof *proof) {
	switch (proof->kind) {
		case PF_PROOF_DIRECT:
			print_proof_direct(proof->direct.start, proof->direct.transform);
			break;
	}
}

void print_axiom(PF_Axiom *axiom) {
	printf("AXIOM %s ", axiom->name);
	if (axiom->params) printf("<");
	_print_ident_list(axiom->params);
	if (axiom->params) printf("> ");
	_PF_print_expr(axiom->lhs);
	printf(" = ");
	PF_print_expr(axiom->rhs);
}

void print_theorem(PF_Theorem *theorem) {
	printf("THEOREM %s ", theorem->name);
	if (theorem->params) printf("<");
	_print_ident_list(theorem->params);
	if (theorem->params) printf("> ");
	_PF_print_expr(theorem->lhs);
	printf(" = ");
	PF_print_expr(theorem->rhs);
	print_proof(&theorem->proof);
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
