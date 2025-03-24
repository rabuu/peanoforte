#ifndef AST_H
#define AST_H

#define GENERATE_LIST_TYPE(typ) \
	struct _##typ##List { \
		typ *head; \
		struct _##typ##List *tail; \
	}

typedef char* PF_Ident;

typedef struct _PF_Program PF_Program;
typedef struct _PF_IdentList PF_IdentList;
typedef struct _PF_Axiom PF_Axiom;
typedef struct _PF_AxiomList PF_AxiomList;
typedef struct _PF_Theorem PF_Theorem;
typedef struct _PF_TheoremList PF_TheoremList;
typedef struct _PF_Expr PF_Expr;
typedef struct _PF_ExprList PF_ExprList;
typedef struct _PF_Proof PF_Proof;
typedef struct _PF_ProofNodeExpr PF_ProofNodeExpr;
typedef struct _PF_ProofNodeTransform PF_ProofNodeTransform;

struct _PF_Program {
	PF_AxiomList *axioms;
	PF_TheoremList *theorems;
};

struct _PF_IdentList {
	PF_Ident ident;
	PF_IdentList *tail;
};

struct _PF_Axiom {
	PF_Ident name;
	PF_IdentList *params;
	PF_Expr *lhs;
	PF_Expr *rhs;
};
GENERATE_LIST_TYPE(PF_Axiom);

struct _PF_Theorem {
	PF_Ident name;
	PF_IdentList *params;
	PF_Expr *lhs;
	PF_Expr *rhs;
	PF_Proof *proof;
};
GENERATE_LIST_TYPE(PF_Theorem);

struct _PF_Expr {
	enum {
		PF_EXPR_ZERO,
		PF_EXPR_VAR,
		PF_EXPR_SEXP,
	} kind;
	union {
		PF_Ident var;
		PF_ExprList *sexp;
	};
};
GENERATE_LIST_TYPE(PF_Expr);

struct _PF_Proof {
	enum {
		PF_PROOF_DIRECT,
	} kind;
	union {
		struct {
			PF_Expr *start;
			PF_ProofNodeTransform *transform;
		} direct;
	};
};

struct _PF_ProofNodeExpr {
	PF_Expr *expr;
	PF_ProofNodeTransform *transform;
};

struct _PF_ProofNodeTransform {
	PF_Ident name;
	bool reversed;
	PF_ProofNodeExpr *expr;
};

PF_Program *PF_program(PF_AxiomList *axioms, PF_TheoremList *theorems);
PF_IdentList *PF_ident_list(PF_Ident ident, PF_IdentList *tail);
PF_Axiom *PF_axiom(PF_Ident name, PF_IdentList *params, PF_Expr *lhs, PF_Expr *rhs);
PF_AxiomList *PF_axiom_list(PF_Axiom *axiom, PF_AxiomList *tail);
PF_Theorem *PF_theorem(PF_Ident name, PF_IdentList *params, PF_Expr *lhs, PF_Expr *rhs, PF_Proof *proof);
PF_TheoremList *PF_theorem_list(PF_Theorem *theorem, PF_TheoremList *tail);
PF_Expr *PF_expr_num(int num);
PF_Expr *PF_expr_var(PF_Ident var);
PF_Expr *PF_expr_sexp(PF_ExprList *sexp);
PF_ExprList *PF_expr_list(PF_Expr *expr, PF_ExprList *tail);
PF_Proof *PF_proof_direct(PF_Expr *start, PF_ProofNodeTransform *transform);
PF_ProofNodeExpr *PF_proof_node_expr(PF_Expr *expr, PF_ProofNodeTransform *transform);
PF_ProofNodeTransform *PF_proof_node_transform(PF_Ident name, bool reversed, PF_ProofNodeExpr *expr);

#endif // !AST_H
