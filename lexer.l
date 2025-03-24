%{
#include "parser.h"
%}

%option nounput noinput noyywrap
%option never-interactive

DIGIT [0-9]
IDENT [_]?[a-zA-Z][-a-zA-Z0-9]*

%%

"axiom" { return KW_AXIOM; }
"theorem" { return KW_THEOREM; }
"example" { return KW_EXAMPLE; }
"induction" { return KW_INDUCTION; }
"base" { return KW_BASE; }
"step" { return KW_STEP; }
"todo" { return KW_TODO; }
"by" { return KW_BY; }
"rev" { return KW_REV; }

"(" { return PAREN_OPEN; }
")" { return PAREN_CLOSE; }
"[" { return BRACKET_OPEN; }
"]" { return BRACKET_CLOSE; }
"{" { return CURLY_OPEN; }
"}" { return CURLY_CLOSE; }
"<" { return ANGLE_OPEN; }
">" { return ANGLE_CLOSE; }
"=" { return EQUALS; }

[ \t\n]+ { }

{DIGIT}+ {
	yylval.num = atoi(yytext);
	return NUMBER;
}

{IDENT} {
	size_t len = strlen(yytext);
	char *str = calloc(len + 1, sizeof(char*));
	str = strncpy(str, yytext, len);
	str[len] = '\0';
	yylval.ident = str;
	return IDENT;
}

. {
	printf("** LEX ERROR ** illegal symbol: %s\n", yytext);
	exit(1);
}

%%
