#include "ast.h"
#include "parser.h"
#include "print.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Ident name;
    IdentList *params;
    Expr *lhs;
    Expr *rhs;
} Rule;

typedef struct {
    size_t count;
    Rule rules[];
} Rules;

typedef struct {
    Expr *lhs;
    Expr *rhs;
} InductionRule;

typedef struct {
    Ident param;
    Expr *expr;
} Binding;

typedef struct {
    size_t count;
    Binding bindings[];
} Bindings;

/* forward declarations */
Expr *find_marked_expr(Expr *expr);
void unmark_expr(Expr *expr);
bool expr_matches_pattern(Expr *expr, Expr *pattern, IdentList *params, Bindings *bindings);
Expr *clone_expr_and_replace(Expr *orig, Expr *replacement, Ident param);
bool verify_rule_left(Expr *expr, Expr *pattern, IdentList *params, Bindings *bindings);
bool verify_rule_right(Expr *expr, Expr *marked, Expr *replace, Expr *target, IdentList *params,
                       Bindings *bindings);
bool verify_proof(Proof *proof, IdentList *params, Expr *lhs, Expr *rhs, Rules *rules);

Rules *allocate_rules(size_t len) {
    Rules *rules = malloc(sizeof(Rules) + len * sizeof(Rule));
    rules->count = 0;
    return rules;
}

void add_rule(Rules *rules, Ident name, IdentList *params, Expr *lhs, Expr *rhs) {
    rules->rules[rules->count] = (Rule){
        .name = name,
        .params = params,
        .lhs = lhs,
        .rhs = rhs,
    };
    rules->count++;
}

Rule *find_rule(Ident name, Rules *rules) {
    for (size_t i = 0; i < rules->count; ++i) {
        Rule rule = rules->rules[i];
        if (!strcmp(name, rule.name)) { return &rules->rules[i]; }
    }
    return nullptr;
}

Bindings *allocate_bindings(size_t len) {
    Bindings *bindings = malloc(sizeof(Bindings) + len * sizeof(Binding));
    bindings->count = 0;
    return bindings;
}

void add_binding(Bindings *bindings, Ident param, Expr *expr) {
    bindings->bindings[bindings->count] = (Binding){
        .param = param,
        .expr = expr,
    };
    bindings->count++;
}

Binding *find_binding(Ident name, Bindings *bindings) {
    if (!bindings) { return nullptr; }

    for (size_t i = 0; i < bindings->count; ++i) {
        Binding binding = bindings->bindings[i];
        if (!strcmp(name, binding.param)) { return &bindings->bindings[i]; }
    }
    return nullptr;
}

void debug_bindings(Bindings *bindings) {
    for (size_t i = 0; i < bindings->count; ++i) {
        Binding binding = bindings->bindings[i];
        printf("DEBUG: %s -> ", binding.param);
        print_expr(binding.expr);
    }
}

void warn_more_marked_exprs(ExprList *list) {
    if (!list) { return; }
    if (find_marked_expr(list->head)) {
        printf("** WARN ** More than one subexpression marked: ");
        print_expr(list->head);
        unmark_expr(list->head);
    }
    return warn_more_marked_exprs(list->tail);
}

Expr *find_marked_expr_in_list(ExprList *list) {
    if (!list) { return nullptr; }

    Expr *marked;
    if ((marked = find_marked_expr(list->head))) {
        warn_more_marked_exprs(list->tail);
        return marked;
    }
    return find_marked_expr_in_list(list->tail);
}

Expr *find_marked_expr(Expr *expr) {
    if (!expr) { return nullptr; }

    Expr *found = nullptr;
    if (expr->marked) { found = expr; }

    switch (expr->tag) {
    case EXPR_ZERO:
    case EXPR_VAR:
        break;
    case EXPR_SEXP:
        if (found) {
            warn_more_marked_exprs(expr->sexp);
        } else {
            found = find_marked_expr_in_list(expr->sexp);
        }
        break;
    }

    return found;
}

void unmark_expr_list(ExprList *list) {
    if (!list) { return; }
    unmark_expr(list->head);
    unmark_expr_list(list->tail);
}

void unmark_expr(Expr *expr) {
    if (!expr) { return; }
    if (expr->marked) { expr->marked = false; }

    if (expr->tag == EXPR_SEXP) { unmark_expr_list(expr->sexp); }
}

ExprList *clone_expr_list_and_replace(ExprList *orig, Expr *replacement, Ident param) {
    if (!orig) { return nullptr; }

    Expr *new_head = clone_expr_and_replace(orig->head, replacement, param);
    ExprList *new_tail = clone_expr_list_and_replace(orig->tail, replacement, param);

    return new_expr_list(new_head, new_tail);
}

Expr *clone_expr_and_replace(Expr *orig, Expr *replacement, Ident param) {
    if (!orig) { return nullptr; }

    switch (orig->tag) {
    case EXPR_ZERO:
        return new_expr_zero(orig->marked);
    case EXPR_VAR:
        if (replacement) {
            if (!strcmp(orig->var, param)) {
                return clone_expr_and_replace(replacement, nullptr, nullptr);
            }
        }
        return new_expr_var(strdup(orig->var), orig->marked);
    case EXPR_SEXP:
        ExprList *sexp = clone_expr_list_and_replace(orig->sexp, replacement, param);
        return new_expr_sexp(sexp, orig->marked);
    }

    return nullptr;
}

bool expr_equals(Expr *a, Expr *b) { return expr_matches_pattern(a, b, nullptr, nullptr); }

bool expr_list_matches_pattern(ExprList *expr_list, ExprList *pattern_list, IdentList *params,
                               Bindings *bindings) {
    if (!expr_list) { return !pattern_list; }
    if (!pattern_list) { return !expr_list; }

    if (expr_matches_pattern(expr_list->head, pattern_list->head, params, bindings)) {
        return expr_list_matches_pattern(expr_list->tail, pattern_list->tail, params, bindings);
    }

    return false;
}

bool expr_matches_pattern(Expr *expr, Expr *pattern, IdentList *params, Bindings *bindings) {
    if (!expr) { return !pattern; }
    if (!pattern) { return !expr; }

    switch (pattern->tag) {
    case EXPR_ZERO:
        return expr->tag == EXPR_ZERO;
    case EXPR_VAR:
        Binding *binding;
        if ((binding = find_binding(pattern->var, bindings))) {
            return expr_equals(expr, binding->expr);
        }
        if (ident_list_contains(pattern->var, params)) {
            add_binding(bindings, pattern->var, expr);
            return true;
        }
        if (expr->tag == EXPR_VAR) { return !strcmp(expr->var, pattern->var); }
        break;
    case EXPR_SEXP:
        if (expr->tag == EXPR_SEXP) {
            return expr_list_matches_pattern(expr->sexp, pattern->sexp, params, bindings);
        }
        break;
    }
    return false;
}

bool verify_rule_left_sexp(ExprList *expr_list, ExprList *pattern_list, IdentList *params,
                           Bindings *bindings) {
    if (!expr_list) { return !pattern_list; }
    if (!pattern_list) { return !expr_list; }

    if (!verify_rule_left(expr_list->head, pattern_list->head, params, bindings)) { return false; }
    return verify_rule_left_sexp(expr_list->tail, pattern_list->tail, params, bindings);
}

bool verify_rule_left(Expr *expr, Expr *pattern, IdentList *params, Bindings *bindings) {
    switch (pattern->tag) {
    case EXPR_ZERO:
        return expr->tag == EXPR_ZERO;
    case EXPR_VAR:
        if (ident_list_contains(pattern->var, params)) {
            Binding *existing_binding;
            if ((existing_binding = find_binding(pattern->var, bindings))) {
                return expr_equals(expr, existing_binding->expr);
            }

            add_binding(bindings, pattern->var, expr);
            return true;
        }
        if (expr->tag != EXPR_VAR) { return false; }
        if (strcmp(expr->var, pattern->var)) { return false; }
        return true;
    case EXPR_SEXP:
        if (expr->tag != EXPR_SEXP) { return false; }
        return verify_rule_left_sexp(expr->sexp, pattern->sexp, params, bindings);
    }
    return false;
}

bool verify_rule_right_sexp(ExprList *expr_list, Expr *marked, Expr *replace, ExprList *target_list,
                            IdentList *params, Bindings *bindings) {
    if (!expr_list) { return !target_list; }
    if (!target_list) { return !expr_list; }

    if (!verify_rule_right(expr_list->head, marked, replace, target_list->head, params, bindings)) {
        return false;
    }
    return verify_rule_right_sexp(expr_list->tail, marked, replace, target_list->tail, params,
                                  bindings);
}

bool verify_rule_right(Expr *expr, Expr *marked, Expr *replace, Expr *target, IdentList *params,
                       Bindings *bindings) {
    if (expr == marked) { return expr_matches_pattern(target, replace, params, bindings); }

    switch (expr->tag) {
    case EXPR_ZERO:
    case EXPR_VAR:
        return expr_equals(expr, target);
    case EXPR_SEXP:
        if (target->tag != EXPR_SEXP) { return false; }
        return verify_rule_right_sexp(expr->sexp, marked, replace, target->sexp, params, bindings);
    }

    return false;
}

bool verify_transform(Expr *expr, Transform *transform, Expr *rhs, Rules *rules,
                      InductionRule *induction_rule) {
    if (!transform) {
        if (!expr_equals(expr, rhs)) {
            printf("** ERROR ** Transformed expression is not RHS.\n");
            return false;
        }
        return true;
    }

    switch (transform->tag) {
    case TRANSFORM_NAMED:
        Expr *marked = find_marked_expr(expr);
        if (!marked) { marked = expr; }

        Rule *rule = find_rule(transform->name, rules);
        if (!rule) {
            printf("** ERROR ** There is no rule with name %s.", transform->name);
            return false;
        }

        size_t params_count = ident_list_count(rule->params);
        Bindings *bindings = allocate_bindings(params_count);

        Expr *rule_lhs = transform->reversed ? rule->rhs : rule->lhs;
        Expr *rule_rhs = transform->reversed ? rule->lhs : rule->rhs;

        if (!verify_rule_left(marked, rule_lhs, rule->params, bindings)) {
            printf("** ERROR ** Expression doesn't match rule.\n");
            printf("EXPRESSION: ");
            print_expr(marked);
            printf("PATTERN: ");
            print_expr(rule_lhs);
            debug_bindings(bindings);
            free(bindings);
            return false;
        }

        Expr *target = transform->target ? transform->target : rhs;
        if (!verify_rule_right(expr, marked, rule_rhs, target, rule->params, bindings)) {
            printf("** ERROR ** Transformed expression doesn't match target.\n");
            printf("EXPRESSION: ");
            print_expr(expr);
            printf("PATTERN: ");
            print_expr(rule_rhs);
            printf("TARGET: ");
            print_expr(target);
            debug_bindings(bindings);
            free(bindings);
            return false;
        }

        free(bindings);
        break;
    case TRANSFORM_INDUCTION:
        if (!induction_rule) {
            printf("** ERROR ** Can't apply induction in a direct proof.\n");
            return false;
        }

        marked = find_marked_expr(expr);
        if (!marked) { marked = expr; }

        if (!expr_equals(marked, induction_rule->lhs)) {
            printf("** ERROR ** Expression doesn't match induction rule.\n");
            print_expr(marked);
            print_expr(induction_rule->lhs);
            return false;
        }

        target = transform->target ? transform->target : rhs;
        if (!verify_rule_right(expr, marked, induction_rule->rhs, target, nullptr, nullptr)) {
            printf("** ERROR ** Transformed expression doesn't match induction "
                   "target.\n");
            print_expr(expr);
            print_expr(induction_rule->rhs);
            print_expr(target);
            return false;
        }
        break;
    case TRANSFORM_TODO:
        break;
    }

    if (transform->target) {
        return verify_transform(transform->target, transform->next, rhs, rules, induction_rule);
    }

    return true;
}

bool verify_proof_direct(Direct *direct, Expr *lhs, Expr *rhs, Rules *rules,
                         InductionRule *induction_rule) {
    Expr *start = direct->start;
    if (start) {
        if (!expr_equals(start, lhs)) {
            printf("** ERROR ** Starting expression does not equal LHS.\n");
            print_expr(start);
            print_expr(lhs);
            return false;
        }
    } else {
        start = lhs;
    }

    return verify_transform(start, direct->transform, rhs, rules, induction_rule);
}

bool verify_proof_induction(Induction *induction, IdentList *params, Expr *lhs, Expr *rhs,
                            Rules *rules) {
    if (!ident_list_contains(induction->var, params)) {
        printf("** ERROR ** Induction over %s not possible.", induction->var);
        return false;
    }

    InductionRule induction_rule = (InductionRule){
        .lhs = lhs,
        .rhs = rhs,
    };

    Expr *base_lhs = clone_expr_and_replace(lhs, new_expr_zero(false), induction->var);
    Expr *base_rhs = clone_expr_and_replace(rhs, new_expr_zero(false), induction->var);
    if (!verify_proof_direct(&induction->base, base_lhs, base_rhs, rules, nullptr)) {
        return false;
    }

    Expr *step_lhs = clone_expr_and_replace(
        lhs, new_expr_succ(new_expr_var(strdup(induction->var), false), false), induction->var);
    Expr *step_rhs = clone_expr_and_replace(
        rhs, new_expr_succ(new_expr_var(strdup(induction->var), false), false), induction->var);

    if (!verify_proof_direct(&induction->step, step_lhs, step_rhs, rules, &induction_rule)) {
        return false;
    }

    return true;
}

bool verify_proof(Proof *proof, IdentList *params, Expr *lhs, Expr *rhs, Rules *rules) {
    switch (proof->tag) {
    case PROOF_DIRECT:
        return verify_proof_direct(&proof->direct, lhs, rhs, rules, nullptr);
    case PROOF_INDUCTION:
        return verify_proof_induction(&proof->induction, params, lhs, rhs, rules);
    }
    return false;
}

bool verify_define(Define *define, Rules *rules) {
    if (find_rule(define->name, rules)) {
        printf("** ERROR ** Duplicate name %s.\n", define->name);
        return false;
    }

    if (find_marked_expr(define->lhs)) {
        printf("WARN: LHS of define %s contains mark: ", define->name);
        print_expr(define->lhs);
        unmark_expr(define->lhs);
    }
    if (find_marked_expr(define->rhs)) {
        printf("WARN: RHS of define %s contains mark: ", define->name);
        print_expr(define->rhs);
        unmark_expr(define->rhs);
    }

    add_rule(rules, define->name, define->params, define->lhs, define->rhs);

    return true;
}

bool verify_theorem(Theorem *theorem, Rules *rules) {
    if (find_rule(theorem->name, rules)) {
        printf("** ERROR ** Duplicate name %s.\n", theorem->name);
        return false;
    }

    if (find_marked_expr(theorem->lhs)) {
        printf("WARN: LHS of theorem %s contains mark: ", theorem->name);
        print_expr(theorem->lhs);
        unmark_expr(theorem->lhs);
    }
    if (find_marked_expr(theorem->rhs)) {
        printf("WARN: RHS of theorem %s contains mark: ", theorem->name);
        print_expr(theorem->rhs);
        unmark_expr(theorem->rhs);
    }

    if (!verify_proof(&theorem->proof, theorem->params, theorem->lhs, theorem->rhs, rules)) {
        return false;
    }

    add_rule(rules, theorem->name, theorem->params, theorem->lhs, theorem->rhs);

    return true;
}

bool verify_example(Example *example, Rules *rules) {
    if (find_marked_expr(example->lhs)) {
        printf("WARN: LHS of an example contains mark: ");
        print_expr(example->lhs);
        unmark_expr(example->lhs);
    }
    if (find_marked_expr(example->rhs)) {
        printf("WARN: RHS of an example contains mark: ");
        print_expr(example->rhs);
        unmark_expr(example->rhs);
    }

    return verify_proof(&example->proof, nullptr, example->lhs, example->rhs, rules);
}

size_t count_rules(Program *program) {
    if (!program) { return 0; }
    size_t plus = 0;
    switch (program->toplevel.tag) {
    case TOPLEVEL_DEFINE:
    case TOPLEVEL_THEOREM:
        plus = 1;
        break;
    default:
        break;
    }

    return plus + count_rules(program->rest);
}

bool verify_program(Program *program, Rules *rules) {
    if (!program) { return true; }

    switch (program->toplevel.tag) {
    case TOPLEVEL_DEFINE:
        if (!verify_define(&program->toplevel.define, rules)) { return false; }
        break;
    case TOPLEVEL_THEOREM:
        if (!verify_theorem(&program->toplevel.theorem, rules)) { return false; }
        break;
    case TOPLEVEL_EXAMPLE:
        if (!verify_example(&program->toplevel.example, rules)) { return false; }
        break;
    }

    return verify_program(program->rest, rules);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("** ERROR ** Please provide a filename.\n");
        return 1;
    }

    Program *program;
    int parse_error = parse(argv[1], &program);

    if (false) {
        printf("\n** DEBUG PRINT **\n-------------------\n");
        print_program(program);
    }

    if (parse_error) {
        free_program(program);
        return parse_error;
    }

    size_t rule_count = count_rules(program);
    Rules *rules = allocate_rules(rule_count);

    int status = verify_program(program, rules) ? 0 : 1;
    if (!status) { printf("correct.\n"); }

    free_program(program);
    free(rules);

    return status;
}
