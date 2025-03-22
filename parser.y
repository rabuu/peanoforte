%{
 #include <stdio.h>
 #include "lexer.h"
 void yyerror(const char *msg);
%}

%union {
   int num;
}

%define parse.error verbose
%locations

%start input

%token KW_AXIOM KW_THEOREM KW_EXAMPLE KW_INDUCTION KW_BASE KW_STEP KW_TODO KW_SUCC
%token PAREN_OPEN PAREN_CLOSE BRACKET_OPEN BRACKET_CLOSE CURLY_OPEN CURLY_CLOSE
%token COLON EQUALS APPLY APPLY_BACKWARDS

%token <num> NUMBER
%token NAME

%%
input: /* nothing */ | input axiom_definition;
axiom_definition: KW_AXIOM parameters COLON CURLY_OPEN CURLY_CLOSE;
parameters: /* nothing */ | parameters NAME;
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
