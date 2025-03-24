%{
 #include <stdio.h>
 #include "lexer.h"
 #include "ast.h"
 void yyerror(const char *msg);
 PF_Program *PF_ast;
%}

%code requires { #include "ast.h" }
%code provides { int PF_parse(char *filename, PF_Program **ast); }

%union {
   int num;
   PF_Ident ident;
   PF_IdentList *ident_list;
   PF_Program *program;
   PF_Axiom *axiom;
   PF_AxiomList *axiom_list;
   PF_Theorem *theorem;
   PF_TheoremList *theorem_list;
   PF_Expr *expr;
   PF_ExprList *expr_list;
   PF_Proof *proof;
}

%define parse.error verbose
%locations

%start program

%token KW_AXIOM KW_THEOREM KW_EXAMPLE KW_INDUCTION KW_BASE KW_STEP KW_TODO KW_BY KW_REV
%token PAREN_OPEN PAREN_CLOSE BRACKET_OPEN BRACKET_CLOSE
%token CURLY_OPEN CURLY_CLOSE ANGLE_OPEN ANGLE_CLOSE EQUALS

%token <num> NUMBER
%token <ident> IDENT

%type <program> program;
%type <axiom_list> axiom_section;
%type <axiom> axiom_definition;
%type <theorem_list> theorem_section;
%type <theorem> theorem_definition;
%type <ident_list> parameters;
%type <proof> proof;
%type <expr> expr;
%type <expr_list> expr_list;

%%
program:
	axiom_section theorem_section { $$ = PF_program($1, $2); PF_ast = $$; };

axiom_section:
	/* empty */ { $$ = nullptr; }
|   axiom_definition axiom_section { $$ = PF_axiom_list($1, $2); };

axiom_definition:
	KW_AXIOM IDENT expr EQUALS expr {
		$$ = PF_axiom($2, nullptr, $3, $5);
}|  KW_AXIOM IDENT ANGLE_OPEN parameters ANGLE_CLOSE expr EQUALS expr {
		$$ = PF_axiom($2, $4, $6, $8);
};

theorem_section:
	/* empty */ { $$ = nullptr; }
|   theorem_definition theorem_section { $$ = PF_theorem_list($1, $2); };

theorem_definition:
	KW_THEOREM IDENT expr EQUALS expr proof {
		$$ = PF_theorem($2, nullptr, $3, $5, $6);
}|  KW_THEOREM IDENT ANGLE_OPEN parameters ANGLE_CLOSE expr EQUALS expr proof {
		$$ = PF_theorem($2, $4, $6, $8, $9);
};

parameters:
	/* empty */ { $$ = nullptr; }
|   IDENT parameters { $$ = PF_ident_list($1, $2); };

proof: /* empty */ { $$ = nullptr; }

expr:
	NUMBER { $$ = PF_expr_num($1); }
|   IDENT { $$ = PF_expr_var($1); }
|   PAREN_OPEN expr_list PAREN_CLOSE { $$ = PF_expr_sexp($2); };

expr_list:
	expr { $$ = PF_expr_list($1, nullptr); }
|   expr expr_list { $$ = PF_expr_list($1, $2); };
%%

int PF_parse(char *filename, PF_Program **ast) {
   yyin = fopen(filename, "r");
   if (yyin == NULL){
      printf("ERROR: can't read file %s\n", filename);
      exit(1);
   }

   int success = yyparse();
   *ast = PF_ast;
   return success;
}

void yyerror(const char *msg) {
   printf("** ERROR ** line %d: %s\n", yylloc.first_line, msg);
}
