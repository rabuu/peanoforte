%{
#include "parser.h"
%}

%option nounput noinput noyywrap

DIGIT [0-9]
NAME [a-zA-Z][a-zA-Z0-9-]*

%%

"axiom" { return KW_AXIOM; }
"theorem" { return KW_THEOREM; }
"example" { return KW_EXAMPLE; }
"induction" { return KW_INDUCTION; }
"base" { return KW_BASE; }
"step" { return KW_STEP; }
"todo" { return KW_TODO; }
"succ" { return KW_SUCC; }

"(" { return PAREN_OPEN; }
")" { return PAREN_CLOSE; }
"[" { return BRACKET_OPEN; }
"]" { return BRACKET_CLOSE; }
"{" { return CURLY_OPEN; }
"}" { return CURLY_CLOSE; }

":" { return COLON; }
"=" { return EQUALS; }
"@" { return APPLY; }
"$" { return APPLY_BACKWARDS; }

[ \t\n]+ { }

{DIGIT}+ {
	yylval.num = atoi(yytext);
	return NUMBER;
}

{NAME} { return NAME; }

%%
