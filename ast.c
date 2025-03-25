#include "ast.h"

#include <stdlib.h>
#include <string.h>

Program *new_program(TopLevel toplevel, Program *rest) {
	Program *program = malloc(sizeof(Program));
	program->toplevel = toplevel;
	program->rest = rest;
	return program;
}

TopLevel new_toplevel_axiom(Axiom axiom) {
	return (TopLevel) {
		.tag = TOPLEVEL_AXIOM,
		.axiom = axiom,
	};
}

TopLevel new_toplevel_theorem(Theorem theorem) {
	return (TopLevel) {
		.tag = TOPLEVEL_THEOREM,
		.theorem = theorem,
	};
}

TopLevel new_toplevel_example(Example example) {
	return (TopLevel) {
		.tag = TOPLEVEL_EXAMPLE,
		.example = example,
	};
}

IdentList *new_ident_list(Ident ident, IdentList *tail) {
	IdentList *idents = malloc(sizeof(IdentList));
	idents->ident = ident;
	idents->tail = tail;
	return idents;
}

Axiom new_axiom(Ident name, IdentList *params, Expr *lhs, Expr *rhs) {
	return (Axiom) {
		.name = name,
		.params = params,
		.lhs = lhs,
		.rhs = rhs,
	};
}

Theorem new_theorem(Ident name, IdentList *params, Expr *lhs, Expr *rhs, Proof *proof) {
	return (Theorem) {
		.name = name,
		.params = params,
		.lhs = lhs,
		.rhs = rhs,
		.proof = proof,
	};
}

Example new_example(Expr *lhs, Expr *rhs, Proof *proof) {
	return (Example) {
		.lhs = lhs,
		.rhs = rhs,
		.proof = proof,
	};
}

Expr *new_expr_num(int num, bool marked) {
	Expr *expr = malloc(sizeof(Expr));
	if (num == 0) {
		expr->tag = EXPR_ZERO;
	} else {
		expr->tag = EXPR_SEXP;
		ExprList *rec = new_expr_list(new_expr_num(num - 1, false), nullptr);
		ExprList *sexp = new_expr_list(new_expr_var("succ", false), rec);
		expr->sexp = sexp;
	}
	expr->marked = marked;
	return expr;
}

Expr *new_expr_var(Ident var, bool marked) {
	Expr *expr = malloc(sizeof(Expr));
	expr->tag = EXPR_VAR;
	expr->var = var;
	expr->marked = marked;
	return expr;
}

Expr *new_expr_sexp(ExprList *sexp, bool marked) {
	Expr *expr = malloc(sizeof(Expr));
	expr->tag = EXPR_SEXP;
	expr->sexp = sexp;
	expr->marked = marked;
	return expr;
}

Expr *clone_expr(Expr *original) {
	if (!original) return nullptr;

	Expr *new = malloc(sizeof(Expr));
	new->tag = original->tag;

	switch (original->tag) {
		case EXPR_ZERO: break;
		case EXPR_VAR:
			/* new->var = strdup(original->var); */
			new->var = original->var;
			break;
		case EXPR_SEXP:
			new->sexp = clone_expr_list(original->sexp);
			break;
	}

	return new;
}

ExprList *new_expr_list(Expr *expr, ExprList *tail) {
	ExprList *exprs = malloc(sizeof(ExprList));
	exprs->head = expr;
	exprs->tail = tail;
	return exprs;
}

ExprList *clone_expr_list(ExprList *original) {
	if (!original) return nullptr;

	ExprList *new = malloc(sizeof(ExprList));
	new->head = clone_expr(original->head);
	new->tail = clone_expr_list(original->tail);

	return new;
}

Proof *new_proof_direct(Expr *start, ProofNodeTransform *transform) {
	Proof *proof = malloc(sizeof(Proof));
	proof->tag = PROOF_DIRECT;
	proof->direct = (ProofDirect) {
		.start = start,
		.transform = transform,
	};
	return proof;
}

Proof *new_proof_induction(Ident var, Proof *base, Proof *step) {
	Proof *proof = malloc(sizeof(Proof));
	proof->tag = PROOF_INDUCTION;
	proof->induction = (ProofInduction) {
		.var = var,
		.base = base,
		.step = step,
	};
	return proof;
}

ProofNodeExpr *new_proof_node_expr(Expr *expr, ProofNodeTransform *transform) {
	ProofNodeExpr *node = malloc(sizeof(ProofNodeExpr));
	node->expr = expr;
	node->transform = transform;
	return node;
}

ProofNodeTransform *new_proof_node_transform(Ident name, bool reversed, ProofNodeExpr *expr) {
	ProofNodeTransform *node = malloc(sizeof(ProofNodeTransform));
	node->tag = PROOF_TRANSFORM_NAMED;
	node->name = name;
	node->reversed = reversed;
	node->expr = expr;
	return node;
}

ProofNodeTransform *new_proof_node_transform_induction(ProofNodeExpr *expr) {
	ProofNodeTransform *node = malloc(sizeof(ProofNodeTransform));
	node->tag = PROOF_TRANSFORM_INDUCTION;
	node->expr = expr;
	return node;
}

ProofNodeTransform *new_proof_node_transform_todo(ProofNodeExpr *expr) {
	ProofNodeTransform *node = malloc(sizeof(ProofNodeTransform));
	node->tag = PROOF_TRANSFORM_TODO;
	node->expr = expr;
	return node;
}
