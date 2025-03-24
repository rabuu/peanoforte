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
   PF_Program *program;
   PF_TopLevel toplevel;
   PF_Axiom axiom;
   PF_Theorem theorem;
   PF_Ident ident;
   PF_IdentList *ident_list;
   PF_Expr *expr;
   PF_ExprList *expr_list;
   PF_Proof proof;
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
%type <toplevel> toplevel;
%type <axiom> axiom;
%type <theorem> theorem;
%type <ident_list> parameters;
%type <proof> proof;
%type <expr> expr;
%type <expr_list> expr_list;

%%
program:
	/* empty */ { $$ = nullptr; }
|   toplevel program { $$ = PF_program($1, $2); PF_ast = $$; };

toplevel:
	axiom { $$ = PF_toplevel_axiom($1); }
|   theorem { $$ = PF_toplevel_theorem($1); };

axiom:
	KW_AXIOM IDENT expr EQUALS expr {
		$$ = PF_axiom($2, nullptr, $3, $5);
}|  KW_AXIOM IDENT ANGLE_OPEN parameters ANGLE_CLOSE expr EQUALS expr {
		$$ = PF_axiom($2, $4, $6, $8);
};

theorem:
	KW_THEOREM IDENT expr EQUALS expr proof {
		$$ = PF_theorem($2, nullptr, $3, $5, $6);
}|  KW_THEOREM IDENT ANGLE_OPEN parameters ANGLE_CLOSE expr EQUALS expr proof {
		$$ = PF_theorem($2, $4, $6, $8, $9);
};

parameters:
	/* empty */ { $$ = nullptr; }
|   IDENT parameters { $$ = PF_ident_list($1, $2); };

proof: CURLY_OPEN CURLY_CLOSE { $$ = PF_proof_direct(nullptr, nullptr); }

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
