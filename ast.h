#ifndef AST_H
#define AST_H

typedef char* ident;

typedef struct _PF_Program PF_Program;
typedef struct _PF_IdentList PF_IdentList;
typedef struct _PF_AxiomList PF_AxiomList;
typedef struct _PF_Axiom PF_Axiom;
typedef struct _PF_Expr PF_Expr;
typedef struct _PF_ExprList PF_ExprList;

struct _PF_Program {
	PF_AxiomList *axioms;
};

struct _PF_IdentList {
	ident ident;
	PF_IdentList *tail;
};

struct _PF_AxiomList {
	PF_Axiom *axiom;
	PF_AxiomList *tail;
};

struct _PF_Axiom {
	ident name;
	PF_IdentList *params;
	PF_Expr *lhs;
	PF_Expr *rhs;
};

struct _PF_Expr {
	enum {
		PF_EXPR_ZERO,
		PF_EXPR_VAR,
		PF_EXPR_SEXP,
	} kind;
	union {
		ident var;
		PF_ExprList *sexp;
	};
};

struct _PF_ExprList {
	PF_Expr *expr;
	PF_ExprList *tail;
};

PF_Program *PF_program(PF_AxiomList *axioms);
PF_IdentList *PF_ident_list(ident ident, PF_IdentList *tail);
PF_Axiom *PF_axiom(ident name, PF_IdentList *params, PF_Expr *lhs, PF_Expr *rhs);
PF_AxiomList *PF_axiom_list(PF_Axiom *axiom, PF_AxiomList *tail);
PF_Expr *PF_expr_num(int num);
PF_Expr *PF_expr_var(ident var);
PF_Expr *PF_expr_sexp(PF_ExprList *sexp);
PF_ExprList *PF_expr_list(PF_Expr *expr, PF_ExprList *tail);

#endif // !AST_H
