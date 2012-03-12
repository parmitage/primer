%{

#include <stdlib.h>
#include <string.h>
#include "pri.h"
#include "y.tab.h"

void yyerror(char *);

%}

%%

";"                                               return SEMICOLON;
"("                                               return LPAREN;
")"                                               return RPAREN;
"["                                               return LSQUARE;
"]"                                               return RSQUARE;
"="                                               return DEF;
"not"                                             return NOT;
">="                                              return GE;
"<="                                              return LE;
"=="                                              return EQ;
"!="                                              return NE;
"and"                                             return AND;
"or"                                              return OR;
"++"                                              return APPEND;
".."                                              return RANGE;
"mod"                                             return MOD;
"&"                                               return B_AND;
"|"                                               return B_OR;
"^"                                               return B_XOR;
"~"                                               return B_NOT;
"<<"                                              return B_LSHIFT;
">>"                                              return B_RSHIFT;
"true"                                            return TRUE;
"false"                                           return FALSE;
"val"                                             return VAL;
"let"                                             return LET;
"in"                                              return IN;
"fun"                                             return LAMBDA;
"->"                                              return DEFINED;
"if"                                              return IF;
"then"                                            return THEN;
"else"                                            return ELSE;
"match"                                           return MATCH;
"with"                                            return WITH;
"_"                                               return ANY;
"is"                                              return IS;
"as"                                              return AS;
"at"                                              return AT;
"::"                                              return CONS;
"head"                                            return HEAD;
"tail"                                            return TAIL;
"show"                                            return SHOW;
"read"                                            return READ;
"rnd"                                             return RND;
"type"                                            return TYPE;
"length"                                          return LENGTH;
"using"                                           return USING;
\'[-<>=+*/&|%{}.;:(){},\[\]! a-zA-Z0-9]\'         { yylval.ival = yytext[1]; return CHAR; }
\"[-<>=+*/&|%{}.;:(){},\[\]! a-zA-Z0-9]*\"        { yylval.sval = strdup(yytext); return STRING; }
\#.*\n?                                           NULL; /* comments */
[a-zA-Z\_][a-zA-Z0-9\_]*                          { yylval.sval = strdup(yytext); return SYMBOL; }
[0-9]+                                            { yylval.ival = atoi(yytext); return INTEGER; }
[0-9]+"."[0-9]+                                   { yylval.fval = atof(yytext); return FLOAT; }
\n                                                ;
[-<>=+*/&|%{}.;:(){},\[\]]                        { return *yytext; }
[ \t]+                                            ; /* ignore whitespace */
.                                                 yyerror("Unknown character");

%%

int yywrap(void)
{
    return 1;
}
