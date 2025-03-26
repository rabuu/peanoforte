#include "ast.h"

#include <stdlib.h>
#include <string.h>

Program *new_program(TopLevel toplevel, Program *rest) {
	Program *program = malloc(sizeof(Program));
	program->toplevel = toplevel;
	program->rest = rest;
	return program;
}

TopLevel new_toplevel_define(Define define) {
	return (TopLevel) {
		.tag = TOPLEVEL_DEFINE,
		.define = define,
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

bool ident_list_contains(Ident ident, IdentList *list) {
	if (!list) return false;
	if (!strcmp(ident, list->ident)) return true;
	return ident_list_contains(ident, list->tail);
}

size_t ident_list_count(IdentList *params) {
	if (!params) return 0;
	return 1 + ident_list_count(params->tail);
}

Define new_define(Ident name, IdentList *params, Expr *lhs, Expr *rhs) {
	return (Define) {
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

ExprList *new_expr_list(Expr *expr, ExprList *tail) {
	ExprList *exprs = malloc(sizeof(ExprList));
	exprs->head = expr;
	exprs->tail = tail;
	return exprs;
}

Proof *new_proof_direct(Expr *start, Transform *transform) {
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

Transform *new_transform_named(Ident name, bool reversed, Expr *target, Transform *next) {
	Transform *transform = malloc(sizeof(Transform));
	transform->tag = TRANSFORM_NAMED;
	transform->name = name;
	transform->reversed = reversed;
	transform->target = target;
	transform->next = next;
	return transform;
}

Transform *new_transform_induction(Expr *target, Transform *next) {
	Transform *transform = malloc(sizeof(Transform));
	transform->tag = TRANSFORM_INDUCTION;
	transform->target = target;
	transform->next = next;
	return transform;
}

Transform *new_transform_todo(Expr *target, Transform *next) {
	Transform *transform = malloc(sizeof(Transform));
	transform->tag = TRANSFORM_TODO;
	transform->target = target;
	transform->next = next;
	return transform;
}
