#ifndef AST_H
#define AST_H

#include <stddef.h>
typedef char *Ident;

typedef struct _IdentList {
    Ident head;
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

typedef struct _Transform {
    enum {
        TRANSFORM_NAMED,
        TRANSFORM_INDUCTION,
        TRANSFORM_TODO,
    } tag;
    Ident name;
    bool reversed;
    Expr *target;
    struct _Transform *next;
} Transform;

typedef struct {
    Expr *start;
    Transform *transform;
} Direct;

typedef struct {
    Ident var;
    Direct base;
    Direct step;
} Induction;

typedef struct {
    enum {
        PROOF_DIRECT,
        PROOF_INDUCTION,
    } tag;
    union {
        Direct direct;
        Induction induction;
    };
} Proof;

typedef struct {
    Ident name;
    IdentList *params;
    Expr *lhs;
    Expr *rhs;
} Define;

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
        TOPLEVEL_DEFINE,
        TOPLEVEL_THEOREM,
        TOPLEVEL_EXAMPLE,
    } tag;
    union {
        Define define;
        Theorem theorem;
        Example example;
    };
} TopLevel;

typedef struct _Program {
    TopLevel toplevel;
    struct _Program *rest;
} Program;

Program *new_program(TopLevel toplevel, Program *rest);
void free_program(Program *program);
TopLevel new_toplevel_define(Define define);
TopLevel new_toplevel_theorem(Theorem theorem);
TopLevel new_toplevel_example(Example example);
void free_toplevel(TopLevel *toplevel);
Define new_define(Ident name, IdentList *params, Expr *lhs, Expr *rhs);
void free_define(Define *define);
Theorem new_theorem(Ident name, IdentList *params, Expr *lhs, Expr *rhs, Proof proof);
void free_theorem(Theorem *theorem);
Example new_example(Expr *lhs, Expr *rhs, Proof proof);
void free_example(Example *example);
IdentList *new_ident_list(Ident ident, IdentList *tail);
void free_ident_list(IdentList *ident_list);
size_t ident_list_count(IdentList *list);
bool ident_list_contains(Ident ident, IdentList *list);
Expr *new_expr_zero(bool marked);
Expr *new_expr_num(int num, bool marked);
Expr *new_expr_var(Ident var, bool marked);
Expr *new_expr_sexp(ExprList *sexp, bool marked);
Expr *new_expr_succ(Expr *inner, bool marked);
void free_expr(Expr *expr);
ExprList *new_expr_list(Expr *expr, ExprList *tail);
void free_expr_list(ExprList *expr_list);
Direct new_direct(Expr *start, Transform *transform);
void free_direct(Direct *direct);
Induction new_induction(Ident var, Direct base, Direct step);
void free_induction(Induction *induction);
Proof new_proof_direct(Direct direct);
Proof new_proof_induction(Induction induction);
void free_proof(Proof *proof);
Transform *new_transform_named(Ident name, bool reversed, Expr *target, Transform *next);
Transform *new_transform_induction(Expr *target, Transform *next);
Transform *new_transform_todo(Expr *target, Transform *next);
void free_transform(Transform *transform);

#endif // !AST_H
