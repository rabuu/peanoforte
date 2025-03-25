#include "print.h"
#include "parser.h"
#include "ast.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* forward declarations */
Expr *find_marked_expr(Expr *expr);
void unmark_expr(Expr *expr);

typedef struct {
	Ident name;
	IdentList *params;
	Expr *lhs;
	Expr *rhs;
} RuleMap;

typedef struct {
	size_t count;
	RuleMap rules[];
} Rules;

Rules *allocate_rules(size_t len) {
	Rules *rules = malloc(sizeof(Rules) + len * sizeof(RuleMap));
	rules->count = 0;
	return rules;
}

void add_rule(Rules *rules, Ident name, IdentList *params, Expr *lhs, Expr *rhs) {
	rules->rules[rules->count] = (RuleMap) {
		.name = name,
		.params = params,
		.lhs = lhs,
		.rhs = rhs,
	};
	rules->count++;
}

bool rules_contain_name(Rules *rules, Ident name) {
	for (size_t i = 0; i < rules->count; ++i) {
		RuleMap rule = rules->rules[i];
		if (!strcmp(name, rule.name))
			return true;
	}
	return false;
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

void assert_unique_name(Ident name, Rules *rules) {
	if (rules_contain_name(rules, name)) {
		printf("ERROR: Duplicate rule name %s\n", name);
		exit(1);
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
	printf("DEBUG: We have %lu rules.\n", rule_count);

	Rules *rules = allocate_rules(rule_count);

	verify_program(program, rules);
	printf("SUCCESS\n");

	return 0;
}
