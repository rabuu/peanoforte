#include "ast.h"

#include <stdlib.h>

PF_Program *PF_program(PF_TopLevel toplevel, PF_Program *rest) {
	PF_Program *program = malloc(sizeof(PF_Program));
	program->toplevel = toplevel;
	program->rest = rest;
	return program;
}

PF_TopLevel PF_toplevel_axiom(PF_Axiom axiom) {
	return (PF_TopLevel) {
		.kind = PF_TOPLEVEL_AXIOM,
		.axiom = axiom,
	};
}

PF_TopLevel PF_toplevel_theorem(PF_Theorem theorem) {
	return (PF_TopLevel) {
		.kind = PF_TOPLEVEL_THEOREM,
		.theorem = theorem,
	};
}

PF_IdentList *PF_ident_list(PF_Ident ident, PF_IdentList *tail) {
	PF_IdentList *idents = malloc(sizeof(PF_IdentList));
	idents->ident = ident;
	idents->tail = tail;
	return idents;
}

PF_Axiom PF_axiom(PF_Ident name, PF_IdentList *params, PF_Expr *lhs, PF_Expr *rhs) {
	return (PF_Axiom) {
		.name = name,
		.params = params,
		.lhs = lhs,
		.rhs = rhs,
	};
}

PF_Theorem PF_theorem(PF_Ident name, PF_IdentList *params, PF_Expr *lhs, PF_Expr *rhs, PF_Proof proof) {
	return (PF_Theorem) {
		.name = name,
		.params = params,
		.lhs = lhs,
		.rhs = rhs,
		.proof = proof,
	};
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

PF_Proof PF_proof_direct(PF_Expr *start, PF_ProofNodeTransform *transform) {
	return (PF_Proof) {
		.kind = PF_PROOF_DIRECT,
		.direct = {
			.start = start,
			.transform = transform,
		},
	};
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
