%{
 #include <stdio.h>
 #include "lexer.h"
 #include "ast.h"
 void yyerror(const char *msg);
 PF_Program *PF_ast;
%}

%code requires { #include "ast.h" }
%code provides { PF_Program *PF_parse(char *filename); }

%union {
   int num;
   ident ident;
   PF_IdentList *ident_list;
   PF_Program *program;
   PF_AxiomList *axiom_list;
   PF_Axiom *axiom;
   PF_Expr *expr;
   PF_ExprList *expr_list;
}

%define parse.error verbose
%locations

%start program

%token KW_AXIOM KW_THEOREM KW_EXAMPLE KW_INDUCTION KW_BASE KW_STEP KW_TODO KW_SUCC KW_BY KW_REV
%token PAREN_OPEN PAREN_CLOSE BRACKET_OPEN BRACKET_CLOSE CURLY_OPEN CURLY_CLOSE
%token COLON EQUALS

%token <num> NUMBER
%token <ident> IDENT

%type <program> program;
%type <axiom_list> axiom_section;
%type <axiom> axiom_definition;
%type <ident_list> parameters;
%type <expr> expr;
%type <expr_list> expr_list;

%%
program: axiom_section { $$ = PF_program($1); PF_ast = $$; };

axiom_section: /* empty */ { $$ = nullptr; }
             | axiom_definition axiom_section { $$ = PF_axiom_list($1, $2); }

axiom_definition: KW_AXIOM IDENT parameters COLON expr EQUALS expr { $$ = PF_axiom($2, $3, $5, $7); }

parameters: /* empty */ { $$ = nullptr; }
          | IDENT parameters { $$ = PF_ident_list($1, $2); }
          ;

expr: NUMBER { $$ = PF_expr_num($1); }
    | IDENT { $$ = PF_expr_var($1); }
    | PAREN_OPEN expr_list PAREN_CLOSE { $$ = PF_expr_sexp($2); }
    ;

expr_list: expr { $$ = PF_expr_list($1, nullptr); }
         | expr expr_list { $$ = PF_expr_list($1, $2); }
         ;
%%

PF_Program *PF_parse(char *filename) {
   yyin = fopen(filename, "r");
   if (yyin == NULL){
      printf("ERROR: can't read file %s\n", filename);
      exit(1);
   }

   if (yyparse()) {
      printf("ERROR: failed to parse file %s.\n", filename);
   }

   printf("SUCCESS: parsed file %s.\n", filename);
   return PF_ast;
}

void yyerror(const char *msg) {
   printf("** ERROR ** line %d: %s\n", yylloc.first_line, msg);
}
