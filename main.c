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
bool expr_equals(Expr *a, Expr *b);
bool match_lhs(Expr *expr, Expr *pattern, IdentList *params, Bindings *bindings);

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
	if (find_marked_expr(list->head)) {
		assert_no_marked_exprs(list->tail);
		return list->head;
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

bool expr_list_equals(ExprList *a, ExprList *b) {
	if (!a) return b == nullptr;
	if (!b) return a == nullptr;

	if (expr_equals(a->head, b->head)) {
		return expr_list_equals(a->tail, b->tail);
	}

	return false;
}

bool expr_equals(Expr *a, Expr *b) {
	if (!a) return b == nullptr;
	if (!b) return a == nullptr;

	switch (a->tag) {
		case EXPR_ZERO: return b->tag == EXPR_ZERO;
		case EXPR_VAR:
			if (b->tag == EXPR_VAR)
				return !strcmp(a->var, b->var);
			else break;
		case EXPR_SEXP:
			if (b->tag == EXPR_SEXP)
				return expr_list_equals(a->sexp, b->sexp);
			else break;
	}

	return false;
}

bool match_sexp(ExprList *expr_list, ExprList *pattern_list, IdentList *params, Bindings *bindings) {
	if (!expr_list) return pattern_list == nullptr;
	if (!pattern_list) return expr_list == nullptr;

	if (!match_lhs(expr_list->head, pattern_list->head, params, bindings)) return false;
	return match_sexp(expr_list->tail, pattern_list->tail, params, bindings);
}

bool match_lhs(Expr *expr, Expr *pattern, IdentList *params, Bindings *bindings) {
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
			return match_sexp(expr->sexp, pattern->sexp, params, bindings);
	}
	return false;
}

void verify_transform(Expr *expr, ProofNodeTransform *transform, Expr *rhs, Rules *rules) {
	if (!transform) {
		if (!expr_equals(expr, rhs)) {
			printf("ERROR: Transformed expression is not RHS.\n");
			exit(1);
		}
		return;
	}

	switch (transform->tag) {
		case PROOF_TRANSFORM_NAMED:
			Expr *marked = find_marked_expr(expr);
			if (!marked) marked = expr;

			Rule *rule = find_rule(rules, transform->name);
			if (!rule) {
				printf("ERROR: There is no rule with name %s.", transform->name);
				exit(1);
			}

			size_t params_count = ident_list_count(rule->params);
			Bindings *bindings = allocate_bindings(params_count);

			if (!match_lhs(marked, rule->lhs, rule->params, bindings)) {
				printf("ERROR: Expression doesn't match.\n");
				print_expr(marked); print_expr(rule->lhs);
				exit(1);
			}

			debug_bindings(bindings);
			break;
		case PROOF_TRANSFORM_INDUCTION: printf("not implemented\n"); exit(2);
		case PROOF_TRANSFORM_TODO:
			if (transform->expr) {
				verify_transform(transform->expr->expr, transform->expr->transform, rhs, rules);
			}
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

void verify_axiom(Axiom *axiom, Rules *rules) {
	assert_unique_name(axiom->name, rules);

	if (find_marked_expr(axiom->lhs)) {
		printf("WARN: LHS of axiom %s contains mark: ", axiom->name);
		print_expr(axiom->lhs);
		unmark_expr(axiom->lhs);
	}
	if (find_marked_expr(axiom->rhs)) {
		printf("WARN: RHS of axiom %s contains mark: ", axiom->name);
		print_expr(axiom->rhs);
		unmark_expr(axiom->rhs);
	}

	add_rule(rules, axiom->name, axiom->params, axiom->lhs, axiom->rhs);
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
		case TOPLEVEL_AXIOM:
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
		case TOPLEVEL_AXIOM:
			verify_axiom(&program->toplevel.axiom, rules);
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
