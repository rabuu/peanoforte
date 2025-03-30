#include "print.h"
#include "ast.h"

#include <stdio.h>

/* forward declarations */
void _print_expr(Expr *expr);
void print_transform(Transform *transform);
void print_proof(Proof *proof);

void _print_ident_list(IdentList *idents) {
    if (!idents) { return; }
    printf("%s", idents->head);
    if (idents->tail) { printf(" "); }
    _print_ident_list(idents->tail);
}

void _print_sexp_inner(ExprList *sexp) {
    if (!sexp) { return; }
    _print_expr(sexp->head);
    if (sexp->tail) { printf(" "); }
    _print_sexp_inner(sexp->tail);
}

void _print_sexp(ExprList *sexp, bool marked) {
    printf(marked ? "[" : "(");
    _print_sexp_inner(sexp);
    printf(marked ? "]" : ")");
}

void _print_expr(Expr *expr) {
    if (!expr) { printf("null expr"); }

    switch (expr->tag) {
    case EXPR_ZERO:
        expr->marked ? printf("[0]") : printf("0");
        break;
    case EXPR_VAR:
        expr->marked ? printf("[%s]", expr->var) : printf("%s", expr->var);
        break;
    case EXPR_SEXP:
        _print_sexp(expr->sexp, expr->marked);
        break;
    }
}

void print_expr(Expr *expr) {
    _print_expr(expr);
    printf("\n");
}

void print_transform(Transform *transform) {
    if (!transform) { return; }
    printf("TRANSFORM");

    switch (transform->tag) {
    case TRANSFORM_NAMED:
        if (transform->reversed) { printf(" (REVERSED)"); }
        printf(": %s\n", transform->name);
        break;
    case TRANSFORM_INDUCTION:
        printf(": INDUCTION\n");
        break;
    case TRANSFORM_TODO:
        printf(": TODO\n");
        break;
    }

    if (transform->target) { print_expr(transform->target); }
    print_transform(transform->next);
}

void print_proof_direct(Direct proof) {
    printf("START: ");
    if (proof.start) {
        print_expr(proof.start);
    } else {
        printf("IMPLIED\n");
    }
    print_transform(proof.transform);
}

void print_proof_induction(Induction proof) {
    printf("INDUCTION BY %s\n", proof.var);
    printf("--- BASE ---:\n");
    print_proof_direct(proof.base);
    printf("--- STEP ---:\n");
    print_proof_direct(proof.step);
}

void print_proof(Proof *proof) {
    switch (proof->tag) {
    case PROOF_DIRECT:
        print_proof_direct(proof->direct);
        break;
    case PROOF_INDUCTION:
        print_proof_induction(proof->induction);
        break;
    }
}

void print_define(Define *define) {
    printf("DEFINE %s ", define->name);
    if (define->params) { printf("<"); }
    _print_ident_list(define->params);
    if (define->params) { printf("> "); }
    _print_expr(define->lhs);
    printf(" = ");
    print_expr(define->rhs);
}

void print_theorem(Theorem *theorem) {
    printf("THEOREM %s ", theorem->name);
    if (theorem->params) { printf("<"); }
    _print_ident_list(theorem->params);
    if (theorem->params) { printf("> "); }
    _print_expr(theorem->lhs);
    printf(" = ");
    print_expr(theorem->rhs);
    print_proof(&theorem->proof);
}

void print_example(Example *example) {
    printf("EXAMPLE ");
    _print_expr(example->lhs);
    printf(" = ");
    print_expr(example->rhs);
    print_proof(&example->proof);
}

void print_toplevel(TopLevel *toplevel) {
    if (!toplevel) { printf("null toplevel\n"); }

    switch (toplevel->tag) {
    case TOPLEVEL_DEFINE:
        print_define(&toplevel->define);
        break;
    case TOPLEVEL_THEOREM:
        print_theorem(&toplevel->theorem);
        break;
    case TOPLEVEL_EXAMPLE:
        print_example(&toplevel->example);
        break;
    }
}

void print_program(Program *program) {
    if (!program) { return; }
    print_toplevel(&program->toplevel);
    if (program->rest) {
        printf("\n");
        print_program(program->rest);
    }
}
