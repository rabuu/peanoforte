#ifndef AST_H
#define AST_H

typedef char* PF_Ident;

typedef struct _PF_IdentList {
	PF_Ident ident;
	struct _PF_IdentList *tail;
} PF_IdentList;

typedef struct _PF_ExprList PF_ExprList;

typedef struct {
	enum {
		PF_EXPR_ZERO,
		PF_EXPR_VAR,
		PF_EXPR_SEXP,
	} kind;
	union {
		PF_Ident var;
		PF_ExprList *sexp;
	};
} PF_Expr;

struct _PF_ExprList {
	PF_Expr *head;
	struct _PF_ExprList *tail;
};

typedef struct _PF_ProofNodeExpr PF_ProofNodeExpr;
typedef struct _PF_ProofNodeTransform PF_ProofNodeTransform;

struct _PF_ProofNodeExpr {
	PF_Expr *expr;
	PF_ProofNodeTransform *transform;
};

struct _PF_ProofNodeTransform {
	PF_Ident name;
	bool reversed;
	PF_ProofNodeExpr *expr;
};

typedef struct {
	enum {
		PF_PROOF_DIRECT,
	} kind;
	union {
		struct {
			PF_Expr *start;
			PF_ProofNodeTransform *transform;
		} direct;
	};
} PF_Proof;

typedef struct {
	PF_Ident name;
	PF_IdentList *params;
	PF_Expr *lhs;
	PF_Expr *rhs;
} PF_Axiom;

typedef struct {
	PF_Ident name;
	PF_IdentList *params;
	PF_Expr *lhs;
	PF_Expr *rhs;
	PF_Proof proof;
} PF_Theorem;

typedef struct {
	enum {
		PF_TOPLEVEL_AXIOM,
		PF_TOPLEVEL_THEOREM,
	} kind;
	union {
		PF_Axiom axiom;
		PF_Theorem theorem;
	};
} PF_TopLevel;

typedef struct _PF_Program {
	PF_TopLevel toplevel;
	struct _PF_Program *rest;
} PF_Program;


PF_Program *PF_program(PF_TopLevel toplevel, PF_Program *rest);
PF_TopLevel PF_toplevel_axiom(PF_Axiom axiom);
PF_TopLevel PF_toplevel_theorem(PF_Theorem theorem);
PF_IdentList *PF_ident_list(PF_Ident ident, PF_IdentList *tail);
PF_Axiom PF_axiom(PF_Ident name, PF_IdentList *params, PF_Expr *lhs, PF_Expr *rhs);
PF_Theorem PF_theorem(PF_Ident name, PF_IdentList *params, PF_Expr *lhs, PF_Expr *rhs, PF_Proof proof);
PF_Expr *PF_expr_num(int num);
PF_Expr *PF_expr_var(PF_Ident var);
PF_Expr *PF_expr_sexp(PF_ExprList *sexp);
PF_ExprList *PF_expr_list(PF_Expr *expr, PF_ExprList *tail);
PF_Proof PF_proof_direct(PF_Expr *start, PF_ProofNodeTransform *transform);
PF_ProofNodeExpr *PF_proof_node_expr(PF_Expr *expr, PF_ProofNodeTransform *transform);
PF_ProofNodeTransform *PF_proof_node_transform(PF_Ident name, bool reversed, PF_ProofNodeExpr *expr);

#endif // !AST_H
