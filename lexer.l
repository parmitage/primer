%{

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "y.tab.h"

void yyerror(char *);

%}

%%

">="							return GE;
"<="							return LE;
"=="							return EQ;
"!="							return NE;
"&&"							return AND;
"||"							return OR;
"%"								return MOD;
"if"							return IF;
"else"							return ELSE;
"def"							return DEF;
"lambda"						return LAMBDA;
"return"						return RETURN;
"true"							return TRUE;
"false"							return FALSE;
"nil"							return NIL;
"end"							return END;
"Head"							return HEAD;
"Tail"							return TAIL;
"Cons"							return CONS;
"Show"							return SHOW;
"Size"							return SIZE;
[a-zA-Z][a-zA-Z0-9]*			{ yylval.sval = strdup(yytext); return SYMBOL; }
[0-9]+							{ yylval.ival = atoi(yytext); return INTEGER; }
[0-9]+"."[0-9]+         		{ yylval.fval = atof(yytext); return FLOAT; }
[-<>=+*/&|%{}.;:(){},\[\]\"]	{ return *yytext; }
[ \t]+							; /* ignore whitespace */
\n								lineno++;
.								yyerror("Unknown character");

%%

int yywrap(void)
{
    return 1;
}
