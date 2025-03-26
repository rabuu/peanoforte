#include "print.h"
#include "parser.h"
#include "ast.h"

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
bool expr_matches_pattern(Expr *expr, Expr *pattern, Bindings *bindings);
bool verify_rule_left(Expr *expr, Expr *pattern, IdentList *params, Bindings *bindings);
bool verify_rule_right(Expr *expr, Expr *marked, Expr *replace, Expr *target, Bindings *bindings);

Rules *allocate_rules(size_t len) {
	Rules *rules = malloc(sizeof(Rules) + len * sizeof(Rule));
	rules->count = 0;
	return rules;
}

void add_rule(Rules *rules, Ident name, IdentList *params, Expr *lhs, Expr *rhs) {
	rules->rules[rules->count] = (Rule) {
		.name = name,
		.params = params,
		.lhs = lhs,
		.rhs = rhs,
	};
	rules->count++;
}

Rule *find_rule(Rules *rules, Ident name) {
	for (size_t i = 0; i < rules->count; ++i) {
		Rule rule = rules->rules[i];
		if (!strcmp(name, rule.name))
			return &rules->rules[i];
	}
	return nullptr;
}

void assert_unique_name(Ident name, Rules *rules) {
	if (find_rule(rules, name)) {
		printf("ERROR: Duplicate rule name %s\n", name);
		exit(1);
	}
}

Bindings *allocate_bindings(size_t len) {
	Bindings *bindings = malloc(sizeof(Bindings) + len * sizeof(Binding));
	bindings->count = 0;
	return bindings;
}

void add_binding(Bindings *bindings, Ident param, Expr *expr) {
	bindings->bindings[bindings->count] = (Binding) {
		.param = param,
		.expr = expr,
	};
	bindings->count++;
}

Binding *find_binding(Ident name, Bindings *bindings) {
	if (!bindings) return nullptr;

	for (size_t i = 0; i < bindings->count; ++i) {
		Binding binding = bindings->bindings[i];
		if (!strcmp(name, binding.param)) {
			return &bindings->bindings[i];
		}
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

void assert_no_marked_exprs(ExprList *list) {
	if (!list) return;
	if (find_marked_expr(list->head)) {
		printf("** ERROR ** Only one subexpression can be marked: ");
		print_expr(list->head);
		exit(1);
	}
	return assert_no_marked_exprs(list->tail);
}

Expr *find_marked_expr_in_list(ExprList *list) {
	if (!list) return nullptr;

	Expr *marked;
	if ((marked = find_marked_expr(list->head))) {
		assert_no_marked_exprs(list->tail);
		return marked;
	}
	return find_marked_expr_in_list(list->tail);
}

Expr *find_marked_expr(Expr *expr) {
	if (!expr) return nullptr;

	Expr *found = nullptr;
	if (expr->marked) found = expr;

	switch (expr->tag) {
		case EXPR_ZERO:
		case EXPR_VAR:
			break;
		case EXPR_SEXP:
			if (found)
				assert_no_marked_exprs(expr->sexp);
			else
				found = find_marked_expr_in_list(expr->sexp);
			break;
	}

	return found;
}

void unmark_expr_list(ExprList *list) {
	if (!list) return;
	unmark_expr(list->head);
	unmark_expr_list(list->tail);
}

void unmark_expr(Expr *expr) {
	if (!expr) return;
	if (expr->marked) expr->marked = false;

	if (expr->tag == EXPR_SEXP) {
		unmark_expr_list(expr->sexp);
	}
}

bool expr_list_matches_pattern(ExprList *expr_list, ExprList *pattern_list, Bindings *bindings) {
	if (!expr_list) return !pattern_list;
	if (!pattern_list) return !expr_list;

	if (expr_matches_pattern(expr_list->head, pattern_list->head, bindings)) {
		return expr_list_matches_pattern(expr_list->tail, pattern_list->tail, bindings);
	}

	return false;
}

bool expr_matches_pattern(Expr *expr, Expr *pattern, Bindings *bindings) {
	if (!expr) return !pattern;
	if (!pattern) return !expr;

	switch (pattern->tag) {
		case EXPR_ZERO:
			return expr->tag == EXPR_ZERO;
		case EXPR_VAR:
			Binding *binding;
			if ((binding = find_binding(pattern->var, bindings))) {
				return expr_matches_pattern(expr, binding->expr, nullptr);
			}
			if (expr->tag == EXPR_VAR) {
				return !strcmp(expr->var, pattern->var);
			}
			break;
		case EXPR_SEXP:
			if (expr->tag == EXPR_SEXP) {
				return expr_list_matches_pattern(expr->sexp, pattern->sexp, bindings);
			}
			break;
	}
	return false;
}

bool expr_equals(Expr *a, Expr *b) {
	return expr_matches_pattern(a, b, nullptr);
}

bool verify_rule_left_sexp(ExprList *expr_list, ExprList *pattern_list, IdentList *params, Bindings *bindings) {
	if (!expr_list) return !pattern_list;
	if (!pattern_list) return !expr_list;

	if (!verify_rule_left(expr_list->head, pattern_list->head, params, bindings)) return false;
	return verify_rule_left_sexp(expr_list->tail, pattern_list->tail, params, bindings);
}

bool verify_rule_left(Expr *expr, Expr *pattern, IdentList *params, Bindings *bindings) {
	switch (pattern->tag) {
		case EXPR_ZERO: return expr->tag == EXPR_ZERO;
		case EXPR_VAR:
			if (ident_list_contains(pattern->var, params)) {
				Binding *existing_binding;
				if ((existing_binding = find_binding(pattern->var, bindings))) {
					return expr_equals(expr, existing_binding->expr);
				}

				add_binding(bindings, pattern->var, expr);
				return true;
			}
			if (expr->tag != EXPR_VAR) return false;
			if (strcmp(expr->var, pattern->var)) return false;
			return true;
		case EXPR_SEXP:
			if (expr->tag != EXPR_SEXP) return false;
			return verify_rule_left_sexp(expr->sexp, pattern->sexp, params, bindings);
	}
	return false;
}

bool verify_rule_right_sexp(ExprList *expr_list, Expr *marked, Expr *replace, ExprList *target_list, Bindings *bindings) {
	if (!expr_list) return !target_list;
	if (!target_list) return !expr_list;

	if (!verify_rule_right(expr_list->head, marked, replace, target_list->head, bindings))
		return false;
	return verify_rule_right_sexp(expr_list->tail, marked, replace, target_list->tail, bindings);
}

bool verify_rule_right(Expr *expr, Expr *marked, Expr *replace, Expr *target, Bindings *bindings) {
	if (expr == marked) {
		return expr_matches_pattern(target, replace, bindings);
	}
	
	switch (expr->tag) {
		case EXPR_ZERO:
		case EXPR_VAR:
			return expr_equals(expr, target);
		case EXPR_SEXP:
			if (target->tag != EXPR_SEXP) return false;
			return verify_rule_right_sexp(expr->sexp, marked, replace, target->sexp, bindings);
	}

	return false;
}

void verify_transform(Expr *expr, Transform *transform, Expr *rhs, Rules *rules) {
	if (!transform) {
		if (!expr_equals(expr, rhs)) {
			printf("ERROR: Transformed expression is not RHS.\n");
			exit(1);
		}
		return;
	}

	switch (transform->tag) {
		case TRANSFORM_NAMED:
			Expr *marked = find_marked_expr(expr);
			if (!marked) marked = expr;

			Rule *rule = find_rule(rules, transform->name);
			if (!rule) {
				printf("ERROR: There is no rule with name %s.", transform->name);
				exit(1);
			}

			size_t params_count = ident_list_count(rule->params);
			Bindings *bindings = allocate_bindings(params_count);

			Expr *rule_lhs = transform->reversed ? rule->rhs : rule->lhs;
			Expr *rule_rhs = transform->reversed ? rule->lhs : rule->rhs;

			if (!verify_rule_left(marked, rule_lhs, rule->params, bindings)) {
				printf("ERROR: Expression doesn't match rule.\n");
				print_expr(marked); print_expr(rule_lhs);
				exit(1);
			}
			/* debug_bindings(bindings); */

			Expr *target = transform->target ? transform->target : rhs;
			if (!verify_rule_right(expr, marked, rule_rhs, target, bindings)) {
				printf("ERROR: Transformed expression doesn't match target.\n");
				print_expr(expr); print_expr(rule_rhs); print_expr(target);
				exit(1);
			}
			break;
		case TRANSFORM_INDUCTION: printf("not implemented\n"); exit(2);
		case TRANSFORM_TODO:
			break;
	}

	if (transform->target) {
		verify_transform(transform->target, transform->next, rhs, rules);
	}
}

void verify_proof_direct(ProofDirect *proof, Expr *lhs, Expr *rhs, Rules *rules) {
	Expr *start = proof->start;
	if (start) {
		if (!expr_equals(start, lhs)) {
			printf("ERROR: Starting expression does not equal LHS.\n");
			print_expr(start);
			print_expr(lhs);
			exit(1);
		}
	} else {
		start = lhs;
	}

	verify_transform(start, proof->transform, rhs, rules);
}

void verify_proof(Proof *proof, IdentList *, Expr *lhs, Expr *rhs, Rules *rules) {
	switch (proof->tag) {
		case PROOF_DIRECT:
			verify_proof_direct(&proof->direct, lhs, rhs, rules);
			break;
		case PROOF_INDUCTION:
			printf("ERROR: Not yet implemented: 'proof by induction'\n");
			exit(2);
	}
}

void verify_define(Define *define, Rules *rules) {
	assert_unique_name(define->name, rules);

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
}

void verify_theorem(Theorem *theorem, Rules *rules) {
	assert_unique_name(theorem->name, rules);

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

	verify_proof(theorem->proof, theorem->params, theorem->lhs, theorem->rhs, rules);

	add_rule(rules, theorem->name, theorem->params, theorem->lhs, theorem->rhs);
}

void verify_example(Example *example, Rules *rules) {
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

	verify_proof(example->proof, nullptr, example->lhs, example->rhs, rules);
}

size_t count_rules(Program *program) {
	if (!program) return 0;
	size_t plus = 0;
	switch (program->toplevel.tag) {
		case TOPLEVEL_DEFINE:
		case TOPLEVEL_THEOREM:
			plus = 1;
			break;
		default: break;
	}

	return plus + count_rules(program->rest);
}

void verify_program(Program *program, Rules *rules) {
	if (!program) return;

	switch (program->toplevel.tag) {
		case TOPLEVEL_DEFINE:
			verify_define(&program->toplevel.define, rules);
			break;
		case TOPLEVEL_THEOREM:
			verify_theorem(&program->toplevel.theorem, rules);
			break;
		case TOPLEVEL_EXAMPLE:
			verify_example(&program->toplevel.example, rules);
			break;
	}

	verify_program(program->rest, rules);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("ERROR: Please provide a filename.\n");
		return 1;
	}

	printf("*** PARSING ***\n----------------\nSOURCE FILE: `%s`\n", argv[1]);
	Program *program; 
	int parse_error = parse(argv[1], &program);
	if (!parse_error) printf("SUCCESS\n");

	if (false) {
		printf("\n*** DEBUG PRINT ***\n-------------------\n");
		print_program(program);
	}
	if (parse_error) return parse_error;

	printf("\n*** VERIFY ***\n----------------\n");

	size_t rule_count = count_rules(program);
	Rules *rules = allocate_rules(rule_count);

	verify_program(program, rules);
	printf("SUCCESS\n");

	return 0;
}
