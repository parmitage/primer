%{

#include <stdlib.h>
#include <string.h>
#include "eval.h"
#include "y.tab.h"

void yyerror(char *);

%}

%%

"not"                                             return NOT;
">="                                              return GE;
"<="                                              return LE;
"=="                                              return EQ;
"!="                                              return NE;
"and"                                             return AND;
"or"                                              return OR;
"++"                                              return APPEND;
"mod"                                             return MOD;
"if"                                              return IF;
"then"                                            return THEN;
"else"                                            return ELSE;
"elif"                                            return ELIF;
"def"                                             return DEF;
"fn"                                              return LAMBDA;
"where"                                           return WHERE;
"true"                                            return TRUE;
"false"                                           return FALSE;
"nil"                                             return NIL;
"end"                                             return END;
"head"                                            return HEAD;
"tail"                                            return TAIL;
"cons"                                            return CONS;
"show"                                            return SHOW;
"type"                                            return TYPE;
"length"                                          return LENGTH;
"nth"                                             return NTH;
\'[-<>=+*/&|%{}.;:(){},\[\]! a-zA-Z0-9]\'         { yylval.ival = yytext[1]; return CHAR; }
\"[-<>=+*/&|%{}.;:(){},\[\]! a-zA-Z0-9]*\"        { yylval.sval = strdup(yytext); return STRING; }
\#.*\n                                            NULL; /* comments */
[a-zA-Z][a-zA-Z0-9]*                              { yylval.sval = strdup(yytext); return SYMBOL; }
[0-9]+                                            { yylval.ival = atoi(yytext); return INTEGER; }
[0-9]+"."[0-9]+                                   { yylval.fval = atof(yytext); return FLOAT; }
\n                                                lineno++;
[-<>=+*/&|%{}.;:(){},\[\]]                        { return *yytext; }
[ \t]+                                            ; /* ignore whitespace */
.                                                 yyerror("Unknown character");

%%

int yywrap(void)
{
    return 1;
}
