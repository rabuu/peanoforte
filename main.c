#include "print.h"
#include "parser.h"
#include "ast.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* forward declarations */
Expr *find_marked_expr(Expr *expr);

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

void check_axiom(Axiom *axiom, Rules *rules) {
	/* check name */
	if (rules_contain_name(rules, axiom->name)) {
		printf("ERROR: Duplicate rule name %s\n", axiom->name);
		exit(1);
	}

	/* check that exprs are unmarked */
	if (find_marked_expr(axiom->lhs)) {
		printf("WARN: LHS of axiom %s contains mark: ", axiom->name);
		print_expr(axiom->lhs);
	}
	if (find_marked_expr(axiom->rhs)) {
		printf("WARN: RHS of axiom %s contains mark: ", axiom->name);
		print_expr(axiom->rhs);
	}

	add_rule(rules, axiom->name, axiom->params, axiom->lhs, axiom->rhs);
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
			check_axiom(&program->toplevel.axiom, rules);
			break;
		case TOPLEVEL_THEOREM:
		case TOPLEVEL_EXAMPLE:
			break;
	}

	verify_program(program->rest, rules);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("ERROR: Please provide a filename.\n");
		return 1;
	}

	printf("*** PARSING *** filename: `%s`\n", argv[1]);
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

	return parse_error;
}
