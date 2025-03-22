#include "ast_printer.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

void print_indent(int depth) {
	for (int i = 0; i < depth; i++) {
		printf("|  ");
	}
	printf("+-");
}

void _print_ident_list(PF_IdentList *idents, int depth) {
	if (!idents) return;
	printf("%s", idents->ident);
	if (idents->tail) printf(", ");
	_print_ident_list(idents->tail, depth);
}

void print_axiom(PF_Axiom *axiom, int depth) {
	if (!axiom) {
		printf("ERROR: Axiom is NULL\n");
		exit(1);
	}

	print_indent(depth);
	printf("AXIOM %s", axiom->name);
	if (axiom->params) printf(" WITH ");
	_print_ident_list(axiom->params, depth);
	printf("\n");
}

void print_axioms(PF_AxiomList *axioms, int depth) {
	if (!axioms) return;
	print_axiom(axioms->axiom, depth);
	print_axioms(axioms->tail, depth);
}

void PF_print_program(PF_Program *prog, int indent) {
	if (!prog) {
		printf("ERROR: Program is NULL\n");
		exit(1);
	}

	print_axioms(prog->axioms, indent);
}
