#include "ast.h"

#include <stdlib.h>

PF_Program *PF_program(PF_AxiomList *axioms) {
	PF_Program *prog = malloc(sizeof(PF_Program));
	prog->axioms = axioms;
	return prog;
}

PF_IdentList *PF_ident_list(ident ident, PF_IdentList *tail) {
	PF_IdentList *idents = malloc(sizeof(PF_IdentList));
	idents->ident = ident;
	idents->tail = tail;
	return idents;
}

PF_Axiom *PF_axiom(ident name, PF_IdentList *params, PF_Expr *lhs, PF_Expr *rhs) {
	PF_Axiom *axiom = malloc(sizeof(PF_Axiom));
	axiom->name = name;
	axiom->params = params;
	axiom->lhs = lhs;
	axiom->rhs = rhs;
	return axiom;
}

PF_AxiomList *PF_axiom_list(PF_Axiom *axiom, PF_AxiomList *tail) {
	PF_AxiomList *axioms = malloc(sizeof(PF_AxiomList));
	axioms->axiom = axiom;
	axioms->tail = tail;
	return axioms;
}

PF_Expr *PF_expr_num(int num) {
	PF_Expr *expr = malloc(sizeof(PF_Expr));
	expr->kind = PF_EXPR_NUM;
	expr->num = num;
	return expr;
}

PF_Expr *PF_expr_var(ident var) {
	PF_Expr *expr = malloc(sizeof(PF_Expr));
	expr->kind = PF_EXPR_NUM;
	expr->var = var;
	return expr;
}

PF_Expr *PF_expr_sexp(PF_ExprList *sexp) {
	PF_Expr *expr = malloc(sizeof(PF_Expr));
	expr->kind = PF_EXPR_NUM;
	expr->sexp = sexp;
	return expr;
}

PF_ExprList *PF_expr_list(PF_Expr *expr, PF_ExprList *tail) {
	PF_ExprList *exprs = malloc(sizeof(PF_ExprList));
	exprs->expr = expr;
	exprs->tail = tail;
	return exprs;
}
