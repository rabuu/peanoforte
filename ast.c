#include "ast.h"

#include <stdlib.h>
#include <string.h>

Program *new_program(TopLevel toplevel, Program *rest) {
	Program *program = malloc(sizeof(Program));
	program->toplevel = toplevel;
	program->rest = rest;
	return program;
}

void free_program(Program *program) {
	if (!program) return;
	free_toplevel(&program->toplevel);
	free_program(program->rest);
	free(program);
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

void free_toplevel(TopLevel *toplevel) {
	if (!toplevel) return;

	switch (toplevel->tag) {
		case TOPLEVEL_DEFINE:
			free_define(&toplevel->define);
			break;
		case TOPLEVEL_THEOREM:
			free_theorem(&toplevel->theorem);
			break;
		case TOPLEVEL_EXAMPLE:
			free_example(&toplevel->example);
			break;
	}
}

Define new_define(Ident name, IdentList *params, Expr *lhs, Expr *rhs) {
	return (Define) {
		.name = name,
		.params = params,
		.lhs = lhs,
		.rhs = rhs,
	};
}

void free_define(Define *define) {
	if (!define) return;
	free(define->name);
	free_ident_list(define->params);
	free_expr(define->lhs);
	free_expr(define->rhs);
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

void free_theorem(Theorem *theorem) {
	if (!theorem) return;
	free(theorem->name);
	free_ident_list(theorem->params);
	free_expr(theorem->lhs);
	free_expr(theorem->rhs);
	free_proof(theorem->proof);
}

Example new_example(Expr *lhs, Expr *rhs, Proof *proof) {
	return (Example) {
		.lhs = lhs,
		.rhs = rhs,
		.proof = proof,
	};
}

void free_example(Example *example) {
	if (!example) return;
	free_expr(example->lhs);
	free_expr(example->rhs);
	free_proof(example->proof);
}

IdentList *new_ident_list(Ident ident, IdentList *tail) {
	IdentList *idents = malloc(sizeof(IdentList));
	idents->head = ident;
	idents->tail = tail;
	return idents;
}

void free_ident_list(IdentList *ident_list) {
	if (!ident_list) return;
	free(ident_list->head);
	free_ident_list(ident_list->tail);
	free(ident_list);
}

bool ident_list_contains(Ident ident, IdentList *list) {
	if (!list) return false;
	if (!strcmp(ident, list->head)) return true;
	return ident_list_contains(ident, list->tail);
}

size_t ident_list_count(IdentList *params) {
	if (!params) return 0;
	return 1 + ident_list_count(params->tail);
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

void free_expr(Expr *expr) {
	if (!expr) return;

	switch (expr->tag) {
		case EXPR_ZERO: break;
		case EXPR_VAR:
			free(expr->var);
			break;
		case EXPR_SEXP:
			free_expr_list(expr->sexp);
			break;
	}

	free(expr);
}

ExprList *new_expr_list(Expr *expr, ExprList *tail) {
	ExprList *exprs = malloc(sizeof(ExprList));
	exprs->head = expr;
	exprs->tail = tail;
	return exprs;
}

void free_expr_list(ExprList *expr_list) {
	if (!expr_list) return;
	free_expr(expr_list->head);
	free_expr_list(expr_list->tail);
	free(expr_list);
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

void free_proof(Proof *proof) {
	if (!proof) return;

	switch (proof->tag) {
		case PROOF_DIRECT:
			free_expr(proof->direct.start);
			free_transform(proof->direct.transform);
			break;
		case PROOF_INDUCTION:
			free(proof->induction.var);
			free_proof(proof->induction.base);
			free_proof(proof->induction.step);
			break;
	}

	free(proof);
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

void free_transform(Transform *transform) {
	if (!transform) return;

	switch (transform->tag) {
		case TRANSFORM_NAMED:
			free(transform->name);
			free_expr(transform->target);
			free_transform(transform->next);
			break;
		case TRANSFORM_INDUCTION:
			free_expr(transform->target);
			free_transform(transform->next);
			break;
		case TRANSFORM_TODO:
			free_expr(transform->target);
			free_transform(transform->next);
			break;
	}

	free(transform);
}
