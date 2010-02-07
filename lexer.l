%{

#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
#include "y.tab.h"

void yyerror(char *);

%}

%%

"!"											return NOT;
">="										return GE;
"<="										return LE;
"=="										return EQ;
"!="										return NE;
"&&"										return AND;
"||"										return OR;
"++"										return APPEND;
".."										return RANGE;
"%"											return MOD;
"if"										return IF;
"iter"										return ITER;
"while"										return WHILE;
"else"										return ELSE;
"def"										return DEF;
"set"										return SET;
"let"										return LET;
"fn"										return LAMBDA;
"true"										return TRUE;
"false"										return FALSE;
"nil"										return NIL;
"end"										return END;
"head"										return HEAD;
"tail"										return TAIL;
"cons"										return CONS;
"show"										return SHOW;
"type"										return TYPE;
\'[-<>=+*/&|%{}.;:(){},\[\]! a-zA-Z0-9]\'	{ yylval.ival = yytext[1]; return CHAR; }
\"[-<>=+*/&|%{}.;:(){},\[\]! a-zA-Z0-9]*\"	{ yylval.sval = strdup(yytext); return STRING; }
[a-zA-Z][a-zA-Z0-9]*						{ yylval.sval = strdup(yytext); return SYMBOL; }
[0-9]+										{ yylval.ival = atoi(yytext); return INTEGER; }
[0-9]+"."[0-9]+         					{ yylval.fval = atof(yytext); return FLOAT; }
\n											lineno++;
[-<>=+*/&|%{}.;:(){},\[\]]					{ return *yytext; }
[ \t]+										; /* ignore whitespace */
.											yyerror("Unknown character");

%%

int yywrap(void)
{
    return 1;
}
