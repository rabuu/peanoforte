#include "ast_printer.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

void _print_ident_list(PF_IdentList *idents) {
	if (!idents) return;
	printf("%s", idents->ident);
	if (idents->tail) printf(", ");
	_print_ident_list(idents->tail);
}

void print_axiom(PF_Axiom *axiom) {
	if (!axiom) {
		printf("ERROR: Axiom is NULL\n");
		exit(1);
	}

	printf("AXIOM %s", axiom->name);
	if (axiom->params) printf(" WITH ");
	_print_ident_list(axiom->params);
	printf("\n");
}

void print_axioms(PF_AxiomList *axioms) {
	if (!axioms) return;
	print_axiom(axioms->axiom);
	print_axioms(axioms->tail);
}

void PF_print_program(PF_Program *prog) {
	if (!prog) {
		printf("ERROR: Program is NULL\n");
		exit(1);
	}

	print_axioms(prog->axioms);
}
