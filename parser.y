%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "eval.h"

  void yyerror(char *s);
  extern FILE *yyin;
  int yylex(void);
  void yyerror(char *s);

  int lineno = 1;

  %}

%union {
  char* sval;
  int ival;
  float fval;
  node *nPtr;
}

%token <sval> SYMBOL STRING
%token <ival> INTEGER CHAR
%token <fval> FLOAT
%token PROG MAIN DEF ASSIGN LAMBDA IF THEN ELSE ELIF COND APPLY
%token GE LE NE EQ NOT AND OR MOD APPEND TRUE FALSE NIL END LIST WHERE
%token HEAD TAIL CONS SHOW TYPE LENGTH NTH

%nonassoc IFX
%nonassoc ELSE

%left NOT
%left AND OR
%left GE LE EQ NE APPEND '>' '<'
%left '+' '-'
%left '*' '/' MOD

%nonassoc UMINUS

%type <nPtr> program stmts stmt exprs expr list symbol

%%

program :
stmts                                                 { temp = mkcons(PROG, 1, $1); }
;

stmts:
stmt                                                  { $$ = $1; }
| stmts stmt                                          { $$ = mkcons(';', 2, $1, $2); }
;

exprs:
expr                                                  { $$ = mkcons(';', 1, $1); }
| expr ',' exprs                                      { $$ = mkcons(';', 2, $1, $3); }
| /* empty */                                         { $$ = mkcons(';', 0); }
;

stmt:
symbol '=' expr                                       { $$ = mkcons(DEF, 2, $1, $3); }
| expr                                                { $$ = $1; }
;

list:
expr                                                  { $$ = mkcons(LIST, 1, $1); }
| expr ',' list                                       { $$ = mkcons(LIST, 2, $1, $3); }
| /* empty list */                                    { $$ = mkcons(LIST, 0); }
;

expr:
INTEGER                                               { $$ = mkint($1); }
| FLOAT                                               { $$ = mkfloat($1); }
| CHAR                                                { $$ = mkchar($1); }
| TRUE                                                { $$ = mkbool(1); }
| FALSE                                               { $$ = mkbool(0); }
| STRING                                              { $$ = mkstr($1); }
| NIL                                                 { $$ = mknil(); }
| symbol                                              { $$ = $1; }
| symbol '(' exprs ')'                                { $$ = mkcons(APPLY, 2, $1, $3); }
| LAMBDA '(' exprs ')' exprs END                      { $$ = mkcons(LAMBDA, 2, $3, $5); }
| LAMBDA '(' exprs ')' exprs WHERE stmts END          { $$ = mkcons(LAMBDA, 3, $3, $5, $7); }
| IF expr THEN exprs %prec IFX END                    { $$ = mkcons(IF, 2, $2, $4); }
| IF expr THEN exprs ELSE exprs END                   { $$ = mkcons(IF, 3, $2, $4, $6); }
| HEAD '(' expr ')'                                   { $$ = mkcons(HEAD, 1, $3); }
| TAIL '(' expr ')'                                   { $$ = mkcons(TAIL, 1, $3); }
| CONS '(' expr ',' expr ')'                          { $$ = mkcons(CONS, 2, $3, $5); }
| SHOW '(' expr ')'                                   { $$ = mkcons(SHOW, 1, $3); }
| TYPE '(' expr ')'                                   { $$ = mkcons(TYPE, 1, $3); }
| LENGTH '(' expr ')'                                 { $$ = mkcons(LENGTH, 1, $3); }
| NTH '(' expr ',' expr ')'                           { $$ = mkcons(NTH, 2, $3, $5); }
| expr '+' expr                                       { $$ = mkcons('+', 2, $1, $3); }
| expr '-' expr                                       { $$ = mkcons('-', 2, $1, $3); }
| expr '*' expr                                       { $$ = mkcons('*', 2, $1, $3); }
| expr '/' expr                                       { $$ = mkcons('/', 2, $1, $3); }
| expr '<' expr                                       { $$ = mkcons('<', 2, $1, $3); }
| expr '>' expr                                       { $$ = mkcons('>', 2, $1, $3); }
| expr GE expr                                        { $$ = mkcons(GE, 2, $1, $3); }
| expr LE expr                                        { $$ = mkcons(LE, 2, $1, $3); }
| expr NE expr                                        { $$ = mkcons(NE, 2, $1, $3); }
| expr EQ expr                                        { $$ = mkcons(EQ, 2, $1, $3); }
| expr AND expr                                       { $$ = mkcons(AND, 2, $1, $3); }
| expr OR expr                                        { $$ = mkcons(OR, 2, $1, $3); }
| expr MOD expr                                       { $$ = mkcons(MOD, 2, $1, $3); }
| expr APPEND expr                                    { $$ = mkcons(APPEND, 2, $1, $3); }
| NOT expr                                            { $$ = mkcons(NOT, 1, $2); }
| '-' expr %prec UMINUS                               { $$ = mkcons('-', 1, $2); }
| '[' list ']'                                        { $$ = $2; }
;

symbol:
SYMBOL                                                { $$ = mksym($1); }
;

%%

void yyerror(char *s)
{
  char* errmsg = malloc(200);
  errmsg[0] = '\0';
  error_log(s, NULL);
}

void parse(char* filename)
{
  if ((yyin = fopen(filename, "r")) == NULL)
    {
      printf("Error reading input file\n");
    }
	
  yyparse();
  ast = temp;
}

node* parsel(char* filename)
{
  if ((yyin = fopen(filename, "r")) == NULL)
    {
      printf("Error reading input file\n");
    }
	
  yyparse();
  return temp->opr.op[0];
}
