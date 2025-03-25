#ifndef AST_H
#define AST_H

typedef char* Ident;

typedef struct _IdentList {
	Ident ident;
	struct _IdentList *tail;
} IdentList;

typedef struct _ExprList ExprList;

typedef struct {
	enum {
		EXPR_ZERO,
		EXPR_VAR,
		EXPR_SEXP,
	} tag;
	union {
		Ident var;
		ExprList *sexp;
	};
	bool marked;
} Expr;

struct _ExprList {
	Expr *head;
	struct _ExprList *tail;
};

typedef struct _ProofNodeExpr ProofNodeExpr;
typedef struct _ProofNodeTransform ProofNodeTransform;

struct _ProofNodeExpr {
	Expr *expr;
	ProofNodeTransform *transform;
};

struct _ProofNodeTransform {
	enum {
		PROOF_TRANSFORM_NAMED,
		PROOF_TRANSFORM_INDUCTION,
		PROOF_TRANSFORM_TODO,
	} tag;
	Ident name;
	bool reversed;
	ProofNodeExpr *expr;
};

typedef struct _Proof Proof;

typedef struct {
	Expr *start;
	ProofNodeTransform *transform;
} ProofDirect;

typedef struct {
	Ident var;
	struct _Proof *base;
	struct _Proof *step;
} ProofInduction;

struct _Proof {
	enum {
		PROOF_DIRECT,
		PROOF_INDUCTION,
	} tag;
	union {
		ProofDirect direct;
		ProofInduction induction;
	};
};

typedef struct {
	Ident name;
	IdentList *params;
	Expr *lhs;
	Expr *rhs;
} Axiom;

typedef struct {
	Ident name;
	IdentList *params;
	Expr *lhs;
	Expr *rhs;
	Proof *proof;
} Theorem;

typedef struct {
	Expr *lhs;
	Expr *rhs;
	Proof *proof;
} Example;

typedef struct {
	enum {
		TOPLEVEL_AXIOM,
		TOPLEVEL_THEOREM,
		TOPLEVEL_EXAMPLE,
	} tag;
	union {
		Axiom axiom;
		Theorem theorem;
		Example example;
	};
} TopLevel;

typedef struct _Program {
	TopLevel toplevel;
	struct _Program *rest;
} Program;


Program *new_program(TopLevel toplevel, Program *rest);
TopLevel new_toplevel_axiom(Axiom axiom);
TopLevel new_toplevel_theorem(Theorem theorem);
TopLevel new_toplevel_example(Example example);
IdentList *new_ident_list(Ident ident, IdentList *tail);
Axiom new_axiom(Ident name, IdentList *params, Expr *lhs, Expr *rhs);
Theorem new_theorem(Ident name, IdentList *params, Expr *lhs, Expr *rhs, Proof *proof);
Example new_example(Expr *lhs, Expr *rhs, Proof *proof);
Expr *new_expr_num(int num, bool marked);
Expr *new_expr_var(Ident var, bool marked);
Expr *new_expr_sexp(ExprList *sexp, bool marked);
ExprList *new_expr_list(Expr *expr, ExprList *tail);
Proof *new_proof_direct(Expr *start, ProofNodeTransform *transform);
Proof *new_proof_induction(Ident var, Proof *base, Proof *step);
ProofNodeExpr *new_proof_node_expr(Expr *expr, ProofNodeTransform *transform);
ProofNodeTransform *new_proof_node_transform(Ident name, bool reversed, ProofNodeExpr *expr);
ProofNodeTransform *new_proof_node_transform_induction(ProofNodeExpr *expr);
ProofNodeTransform *new_proof_node_transform_todo(ProofNodeExpr *expr);

#endif // !AST_H
