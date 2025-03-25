#include "ast.h"
#include "print.h"

#include <stdio.h>
#include <stdlib.h>

Program *program(TopLevel toplevel, Program *rest) {
	Program *program = malloc(sizeof(Program));
	program->toplevel = toplevel;
	program->rest = rest;
	return program;
}

TopLevel toplevel_axiom(Axiom axiom) {
	return (TopLevel) {
		.tag = TOPLEVEL_AXIOM,
		.axiom = axiom,
	};
}

TopLevel toplevel_theorem(Theorem theorem) {
	return (TopLevel) {
		.tag = TOPLEVEL_THEOREM,
		.theorem = theorem,
	};
}

TopLevel toplevel_example(Example example) {
	return (TopLevel) {
		.tag = TOPLEVEL_EXAMPLE,
		.example = example,
	};
}

IdentList *ident_list(Ident ident, IdentList *tail) {
	IdentList *idents = malloc(sizeof(IdentList));
	idents->ident = ident;
	idents->tail = tail;
	return idents;
}

Axiom axiom(Ident name, IdentList *params, Expr *lhs, Expr *rhs) {
	return (Axiom) {
		.name = name,
		.params = params,
		.lhs = lhs,
		.rhs = rhs,
	};
}

Theorem theorem(Ident name, IdentList *params, Expr *lhs, Expr *rhs, Proof *proof) {
	return (Theorem) {
		.name = name,
		.params = params,
		.lhs = lhs,
		.rhs = rhs,
		.proof = proof,
	};
}

Example example(Expr *lhs, Expr *rhs, Proof *proof) {
	return (Example) {
		.lhs = lhs,
		.rhs = rhs,
		.proof = proof,
	};
}

Expr *expr_num(int num, bool marked) {
	Expr *expr = malloc(sizeof(Expr));
	if (num == 0) {
		expr->tag = EXPR_ZERO;
	} else {
		expr->tag = EXPR_SEXP;
		ExprList *rec = expr_list(expr_num(num - 1, false), nullptr);
		ExprList *sexp = expr_list(expr_var("succ", false), rec);
		expr->sexp = sexp;
	}
	expr->marked = marked;
	return expr;
}

Expr *expr_var(Ident var, bool marked) {
	Expr *expr = malloc(sizeof(Expr));
	expr->tag = EXPR_VAR;
	expr->var = var;
	expr->marked = marked;
	return expr;
}

Expr *expr_sexp(ExprList *sexp, bool marked) {
	Expr *expr = malloc(sizeof(Expr));
	expr->tag = EXPR_SEXP;
	expr->sexp = sexp;
	expr->marked = marked;
	return expr;
}

ExprList *expr_list(Expr *expr, ExprList *tail) {
	ExprList *exprs = malloc(sizeof(ExprList));
	exprs->head = expr;
	exprs->tail = tail;
	return exprs;
}

void _expr_assert_not_marked(ExprList *list) {
	if (!list) return;
	if (expr_find_marked(list->head)) {
		printf("** ERROR ** Only one subexpression can be marked: ");
		print_expr(list->head);
		exit(1);
	}
	return _expr_assert_not_marked(list->tail);
}

Expr *_expr_list_find_marked(ExprList *list) {
	if (!list) return nullptr;
	if (expr_find_marked(list->head)) {
		_expr_assert_not_marked(list->tail);
		return list->head;
	}
	return _expr_list_find_marked(list->tail);
}

Expr *expr_find_marked(Expr *expr) {
	if (!expr) return nullptr;

	Expr *found = nullptr;
	if (expr->marked) found = expr;

	switch (expr->tag) {
		case EXPR_ZERO:
		case EXPR_VAR:
			break;
		case EXPR_SEXP:
			if (found)
				_expr_assert_not_marked(expr->sexp);
			else
				found = _expr_list_find_marked(expr->sexp);
			break;
	}

	return found;
}

Proof *proof_direct(Expr *start, ProofNodeTransform *transform) {
	Proof *proof = malloc(sizeof(Proof));
	proof->tag = PROOF_DIRECT;
	proof->direct = (ProofDirect) {
		.start = start,
		.transform = transform,
	};
	return proof;
}

Proof *proof_induction(Ident var, Proof *base, Proof *step) {
	Proof *proof = malloc(sizeof(Proof));
	proof->tag = PROOF_INDUCTION;
	proof->induction = (ProofInduction) {
		.var = var,
		.base = base,
		.step = step,
	};
	return proof;
}

ProofNodeExpr *proof_node_expr(Expr *expr, ProofNodeTransform *transform) {
	ProofNodeExpr *node = malloc(sizeof(ProofNodeExpr));
	node->expr = expr;
	node->transform = transform;
	return node;
}

ProofNodeTransform *proof_node_transform(Ident name, bool reversed, ProofNodeExpr *expr) {
	ProofNodeTransform *node = malloc(sizeof(ProofNodeTransform));
	node->induction = false;
	node->name = name;
	node->reversed = reversed;
	node->expr = expr;
	return node;
}

ProofNodeTransform *proof_node_transform_induction(ProofNodeExpr *expr) {
	ProofNodeTransform *node = malloc(sizeof(ProofNodeTransform));
	node->induction = true;
	node->expr = expr;
	return node;
}
