#include "ast.h"

#include <stdlib.h>

PF_Program *PF_program(PF_AxiomList *axioms, PF_TheoremList *theorems) {
	PF_Program *prog = malloc(sizeof(PF_Program));
	prog->axioms = axioms;
	prog->theorems = theorems;
	return prog;
}

PF_IdentList *PF_ident_list(PF_Ident ident, PF_IdentList *tail) {
	PF_IdentList *idents = malloc(sizeof(PF_IdentList));
	idents->ident = ident;
	idents->tail = tail;
	return idents;
}

PF_Axiom *PF_axiom(PF_Ident name, PF_IdentList *params, PF_Expr *lhs, PF_Expr *rhs) {
	PF_Axiom *axiom = malloc(sizeof(PF_Axiom));
	axiom->name = name;
	axiom->params = params;
	axiom->lhs = lhs;
	axiom->rhs = rhs;
	return axiom;
}

PF_AxiomList *PF_axiom_list(PF_Axiom *axiom, PF_AxiomList *tail) {
	PF_AxiomList *axioms = malloc(sizeof(PF_AxiomList));
	axioms->head = axiom;
	axioms->tail = tail;
	return axioms;
}

PF_Theorem *PF_theorem(PF_Ident name, PF_IdentList *params, PF_Expr *lhs, PF_Expr *rhs, PF_Proof *proof) {
	PF_Theorem *theorem = malloc(sizeof(PF_Theorem));
	theorem->name = name;
	theorem->params = params;
	theorem->lhs = lhs;
	theorem->rhs = rhs;
	theorem->proof = proof;
	return theorem;
}

PF_TheoremList *PF_theorem_list(PF_Theorem *theorem, PF_TheoremList *tail) {
	PF_TheoremList *theorems = malloc(sizeof(PF_TheoremList));
	theorems->head = theorem;
	theorems->tail = tail;
	return theorems;
}

PF_Expr *PF_expr_num(int num) {
	PF_Expr *expr = malloc(sizeof(PF_Expr));
	if (num == 0) {
		expr->kind = PF_EXPR_ZERO;
	} else {
		expr->kind = PF_EXPR_SEXP;
		PF_ExprList *rec = PF_expr_list(PF_expr_num(num - 1), nullptr);
		PF_ExprList *sexp = PF_expr_list(PF_expr_var("succ"), rec);
		expr->sexp = sexp;
	}
	return expr;
}

PF_Expr *PF_expr_var(PF_Ident var) {
	PF_Expr *expr = malloc(sizeof(PF_Expr));
	expr->kind = PF_EXPR_VAR;
	expr->var = var;
	return expr;
}

PF_Expr *PF_expr_sexp(PF_ExprList *sexp) {
	PF_Expr *expr = malloc(sizeof(PF_Expr));
	expr->kind = PF_EXPR_SEXP;
	expr->sexp = sexp;
	return expr;
}

PF_ExprList *PF_expr_list(PF_Expr *expr, PF_ExprList *tail) {
	PF_ExprList *exprs = malloc(sizeof(PF_ExprList));
	exprs->head = expr;
	exprs->tail = tail;
	return exprs;
}

PF_Proof *PF_proof_direct(PF_Expr *start, PF_ProofNodeTransform *transform) {
	PF_Proof *proof = malloc(sizeof(PF_Proof));
	proof->kind = PF_PROOF_DIRECT;
	proof->direct.start = start;
	proof->direct.transform = transform;
	return proof;
}

PF_ProofNodeExpr *PF_proof_node_expr(PF_Expr *expr, PF_ProofNodeTransform *transform) {
	PF_ProofNodeExpr *node = malloc(sizeof(PF_ProofNodeExpr));
	node->expr = expr;
	node->transform = transform;
	return node;
}

PF_ProofNodeTransform *PF_proof_node_transform(PF_Ident name, bool reversed, PF_ProofNodeExpr *expr) {
	PF_ProofNodeTransform *node = malloc(sizeof(PF_ProofNodeTransform));
	node->name = name;
	node->reversed = reversed;
	node->expr = expr;
	return node;
}
