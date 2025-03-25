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
   Proof *proof;
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
%type <proof> proof_direct;
%type <proof> proof_induction;
%type <proof_node_transform> proof_node_transform;
%type <proof_node_expr> proof_node_expr;
%type <expr> expr;
%type <expr_list> expr_list;

%%
program:
  /* empty */ { $$ = nullptr; }
| toplevel program { $$ = new_program($1, $2); program_ast = $$; }
;

toplevel:
  axiom { $$ = new_toplevel_axiom($1); }
| theorem { $$ = new_toplevel_theorem($1); }
| example { $$ = new_toplevel_example($1); }
;

axiom:
  KW_AXIOM IDENT expr EQUALS expr {
    $$ = new_axiom($2, nullptr, $3, $5);
}
| KW_AXIOM IDENT ANGLE_OPEN parameters ANGLE_CLOSE expr EQUALS expr {
    $$ = new_axiom($2, $4, $6, $8);
}
;

theorem:
  KW_THEOREM IDENT expr EQUALS expr proof {
    $$ = new_theorem($2, nullptr, $3, $5, $6);
}
| KW_THEOREM IDENT ANGLE_OPEN parameters ANGLE_CLOSE expr EQUALS expr proof {
    $$ = new_theorem($2, $4, $6, $8, $9);
}
;

parameters:
  /* empty */ { $$ = nullptr; }
| IDENT parameters { $$ = new_ident_list($1, $2); }
;

example:
  KW_EXAMPLE expr EQUALS expr proof {
    $$ = new_example($2, $4, $5);
}
;

proof:
  proof_direct { $$ = $1; }
| proof_induction { $$ = $1; }
;

proof_direct:
  CURLY_OPEN proof_node_transform CURLY_CLOSE {
    $$ = new_proof_direct(nullptr, $2);
}
| CURLY_OPEN expr proof_node_transform CURLY_CLOSE {
    $$ = new_proof_direct($2, $3);
}
;

proof_induction:
  KW_INDUCTION IDENT CURLY_OPEN KW_BASE proof KW_STEP proof CURLY_CLOSE {
    $$ = new_proof_induction($2, $5, $7);
}
;

proof_node_transform:
  /* empty */ { $$ = nullptr; }
| KW_BY IDENT proof_node_expr {
    $$ = new_proof_node_transform($2, false, $3);
}
| KW_BY KW_REV IDENT proof_node_expr {
    $$ = new_proof_node_transform($3, true, $4);
}
| KW_BY KW_INDUCTION proof_node_expr {
    $$ = new_proof_node_transform_induction($3);
}
| KW_TODO proof_node_expr {
    $$ = new_proof_node_transform_todo($2);
}
;

proof_node_expr:
  /* empty */ { $$ = nullptr; }
| expr proof_node_transform { $$ = new_proof_node_expr($1, $2); }
;

expr:
  NUMBER { $$ = new_expr_num($1, false); }
| BRACKET_OPEN NUMBER BRACKET_CLOSE { $$ = new_expr_num($2, true); }
| IDENT { $$ = new_expr_var($1, false); }
| BRACKET_OPEN IDENT BRACKET_CLOSE { $$ = new_expr_var($2, true); }
| PAREN_OPEN expr_list PAREN_CLOSE { $$ = new_expr_sexp($2, false); }
| BRACKET_OPEN expr_list BRACKET_CLOSE { $$ = new_expr_sexp($2, true); }
;

expr_list:
  expr expr { $$ = new_expr_list($1, new_expr_list($2, nullptr)); }
| expr expr_list { $$ = new_expr_list($1, $2); }
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
