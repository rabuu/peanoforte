#include "print.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

/* forward declarations */
void _print_expr(Expr *expr);
void print_node_transform(ProofNodeTransform *transform);
void print_proof(Proof *proof);

void _print_ident_list(IdentList *idents) {
	if (!idents) return;
	printf("%s", idents->ident);
	if (idents->tail) printf(" ");
	_print_ident_list(idents->tail);
}

void _print_sexp_inner(ExprList *sexp) {
	if (!sexp) return;
	_print_expr(sexp->head);
	if (sexp->tail) printf(" ");
	_print_sexp_inner(sexp->tail);
}

void _print_sexp(ExprList *sexp, bool marked) {
	printf(marked ? "[" : "(");
	_print_sexp_inner(sexp);
	printf(marked ? "]" : ")");
}

void _print_expr(Expr *expr) {
	if (!expr) {
		printf("ERROR: Expr is NULL\n");
		exit(1);
	}

	switch (expr->tag) {
		case EXPR_ZERO:
			expr->marked
				? printf("[0]")
				: printf("0");
			break;
		case EXPR_VAR:
			expr->marked
				? printf("[%s]", expr->var)
				: printf("%s", expr->var);
			break;
		case EXPR_SEXP:
			_print_sexp(expr->sexp, expr->marked);
			break;
	}
}

void print_expr(Expr *expr) {
	_print_expr(expr);
	printf("\n");
}

void print_node_expr(ProofNodeExpr *expr) {
	if (!expr) return;
	printf("EXPR: ");
	print_expr(expr->expr);
	if (expr->transform) print_node_transform(expr->transform);
}

void print_node_transform(ProofNodeTransform *transform) {
	if (!transform) return;
	printf("TRANSFORM");

	switch (transform->tag) {
		case PROOF_TRANSFORM_NAMED:
			if (transform->reversed) printf(" (REVERSED)");
			printf(": %s\n", transform->name);
			break;
		case PROOF_TRANSFORM_INDUCTION:
			printf(": INDUCTION\n");
			break;
		case PROOF_TRANSFORM_TODO:
			printf(": TODO\n");
			break;
	}

	if (transform->expr) print_node_expr(transform->expr);
}

void print_proof_direct(ProofDirect proof) {
	printf("START: ");
	if (proof.start) print_expr(proof.start); else printf("IMPLIED\n");
	print_node_transform(proof.transform);
}

void print_proof_induction(ProofInduction proof) {
	printf("INDUCTION BY %s\n", proof.var);
	printf("--- BASE ---:\n");
	print_proof(proof.base);
	printf("--- STEP ---:\n");
	print_proof(proof.step);
}

void print_proof(Proof *proof) {
	switch (proof->tag) {
		case PROOF_DIRECT:
			print_proof_direct(proof->direct);
			break;
		case PROOF_INDUCTION:
			print_proof_induction(proof->induction);
			break;
	}
}

void print_axiom(Axiom *axiom) {
	printf("AXIOM %s ", axiom->name);
	if (axiom->params) printf("<");
	_print_ident_list(axiom->params);
	if (axiom->params) printf("> ");
	_print_expr(axiom->lhs);
	printf(" = ");
	print_expr(axiom->rhs);
}

void print_theorem(Theorem *theorem) {
	printf("THEOREM %s ", theorem->name);
	if (theorem->params) printf("<");
	_print_ident_list(theorem->params);
	if (theorem->params) printf("> ");
	_print_expr(theorem->lhs);
	printf(" = ");
	print_expr(theorem->rhs);
	print_proof(theorem->proof);
}

void print_example(Example *example) {
	printf("EXAMPLE ");
	_print_expr(example->lhs);
	printf(" = ");
	print_expr(example->rhs);
	print_proof(example->proof);
}

void print_toplevel(TopLevel *toplevel) {
	if (!toplevel) {
		printf("ERROR: Toplevel is NULL\n");
		exit(1);
	}

	switch (toplevel->tag) {
		case TOPLEVEL_AXIOM: print_axiom(&toplevel->axiom); break;
		case TOPLEVEL_THEOREM: print_theorem(&toplevel->theorem); break;
		case TOPLEVEL_EXAMPLE: print_example(&toplevel->example); break;
	}
}

void print_program(Program *program) {
	if (!program) return;
	print_toplevel(&program->toplevel);
	if (program->rest) {
		printf("\n");
		print_program(program->rest);
	}
}
