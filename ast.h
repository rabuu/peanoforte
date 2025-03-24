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
	Ident name;
	bool reversed;
	ProofNodeExpr *expr;
};

typedef struct {
	enum {
		PROOF_DIRECT,
	} tag;
	union {
		struct {
			Expr *start;
			ProofNodeTransform *transform;
		} direct;
	};
} Proof;

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
	Proof proof;
} Theorem;

typedef struct {
	Expr *lhs;
	Expr *rhs;
	Proof proof;
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


Program *program(TopLevel toplevel, Program *rest);
TopLevel toplevel_axiom(Axiom axiom);
TopLevel toplevel_theorem(Theorem theorem);
TopLevel toplevel_example(Example example);
IdentList *ident_list(Ident ident, IdentList *tail);
Axiom axiom(Ident name, IdentList *params, Expr *lhs, Expr *rhs);
Theorem theorem(Ident name, IdentList *params, Expr *lhs, Expr *rhs, Proof proof);
Example example(Expr *lhs, Expr *rhs, Proof proof);
Expr *expr_num(int num, bool marked);
Expr *expr_var(Ident var, bool marked);
Expr *expr_sexp(ExprList *sexp, bool marked);
ExprList *expr_list(Expr *expr, ExprList *tail);
Expr *expr_find_marked(Expr *expr);
Proof proof_direct(Expr *start, ProofNodeTransform *transform);
ProofNodeExpr *proof_node_expr(Expr *expr, ProofNodeTransform *transform);
ProofNodeTransform *proof_node_transform(Ident name, bool reversed, ProofNodeExpr *expr);

#endif // !AST_H
