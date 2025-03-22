%{
 #include <stdio.h>
 #include "lexer.h"
 void yyerror(const char *msg);
%}

%union {
   int num;
   char *ident;
}

%define parse.error verbose
%locations

%start input

%token KW_AXIOM KW_THEOREM KW_EXAMPLE KW_INDUCTION KW_BASE KW_STEP KW_TODO KW_SUCC KW_BY KW_REV
%token PAREN_OPEN PAREN_CLOSE BRACKET_OPEN BRACKET_CLOSE CURLY_OPEN CURLY_CLOSE
%token COLON EQUALS

%token <num> NUMBER
%token <ident> IDENT

%%
input: /* empty */ | input axiom_definition;
axiom_definition: KW_AXIOM IDENT parameters COLON expr EQUALS expr;
parameters: /* empty */ | parameters IDENT;

expr: NUMBER
    | IDENT
    | PAREN_OPEN expr_list PAREN_CLOSE
    ;

expr_list: expr | expr_list expr;
%%

int main(int argc, char **argv) {
   if (argc > 1) {
      yyin = fopen(argv[1], "r");
      if (yyin == NULL){
         printf("syntax: %s filename\n", argv[0]);
      }
   }
   yyparse();
   return 0;
}

void yyerror(const char *msg) {
   printf("** ERROR ** line %d: %s\n", yylloc.first_line, msg);
}
