%{
 #include <stdio.h>
 #include "lexer.h"
 #include "ast.h"
 void yyerror(const char *msg);
 Program *program_ast;
%}

%code requires { #include "ast.h" }
%code provides { int parse(char *filename, Program **ast); }

%union {
   int num;
   Program *program;
   TopLevel toplevel;
   Axiom axiom;
   Theorem theorem;
   Example example;
   Ident ident;
   IdentList *ident_list;
   Expr *expr;
   ExprList *expr_list;
   Proof proof;
   ProofNodeExpr *proof_node_expr;
   ProofNodeTransform *proof_node_transform;
}

%define parse.error verbose

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
%type <example> example;
%type <ident_list> parameters;
%type <proof> proof;
%type <proof_node_transform> proof_node_transform;
%type <proof_node_expr> proof_node_expr;
%type <expr> expr;
%type <expr_list> expr_list;

%%
program:
  /* empty */ { $$ = nullptr; }
| toplevel program { $$ = program($1, $2); program_ast = $$; }
;

toplevel:
  axiom { $$ = toplevel_axiom($1); }
| theorem { $$ = toplevel_theorem($1); }
| example { $$ = toplevel_example($1); }
;

axiom:
  KW_AXIOM IDENT expr EQUALS expr {
    $$ = axiom($2, nullptr, $3, $5);
}
| KW_AXIOM IDENT ANGLE_OPEN parameters ANGLE_CLOSE expr EQUALS expr {
    $$ = axiom($2, $4, $6, $8);
}
;

theorem:
  KW_THEOREM IDENT expr EQUALS expr proof {
    $$ = theorem($2, nullptr, $3, $5, $6);
}
| KW_THEOREM IDENT ANGLE_OPEN parameters ANGLE_CLOSE expr EQUALS expr proof {
    $$ = theorem($2, $4, $6, $8, $9);
}
;

parameters:
  /* empty */ { $$ = nullptr; }
| IDENT parameters { $$ = ident_list($1, $2); }
;

example:
  KW_EXAMPLE expr EQUALS expr proof {
    $$ = example($2, $4, $5);
}
;

proof:
  CURLY_OPEN proof_node_transform CURLY_CLOSE {
    $$ = proof_direct(nullptr, $2);
}
| CURLY_OPEN expr proof_node_transform CURLY_CLOSE {
    $$ = proof_direct($2, $3);
}
;

proof_node_transform:
  /* empty */ { $$ = nullptr; }
| KW_BY IDENT proof_node_expr {
    $$ = proof_node_transform($2, false, $3);
}
| KW_BY KW_REV IDENT proof_node_expr {
    $$ = proof_node_transform($3, true, $4);
}
;

proof_node_expr:
  /* empty */ { $$ = nullptr; }
| expr proof_node_transform { $$ = proof_node_expr($1, $2); }
;

expr:
  NUMBER { $$ = expr_num($1, false); }
| BRACKET_OPEN NUMBER BRACKET_CLOSE { $$ = expr_num($2, true); }
| IDENT { $$ = expr_var($1, false); }
| BRACKET_OPEN IDENT BRACKET_CLOSE { $$ = expr_var($2, true); }
| PAREN_OPEN expr_list PAREN_CLOSE { $$ = expr_sexp($2, false); }
| BRACKET_OPEN expr_list BRACKET_CLOSE { $$ = expr_sexp($2, true); }
;

expr_list:
  expr expr { $$ = expr_list($1, expr_list($2, nullptr)); }
| expr expr_list { $$ = expr_list($1, $2); }
;
%%

int parse(char *filename, Program **ast) {
   yyin = fopen(filename, "r");
   if (yyin == NULL){
      printf("ERROR: can't read file %s\n", filename);
      exit(1);
   }

   int success = yyparse();
   *ast = program_ast;
   return success;
}

void yyerror(const char *msg) {
   printf("** PARSE ERROR ** %s\n", msg);
}
