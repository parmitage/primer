%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "interpreter.h"

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
%token PROG DEF ASSIGN LAMBDA IF THEN ELSE FUNCALL MODULECALL USE
%token GE LE NE EQ NOT AND OR MOD APPEND TRUE FALSE NIL END LIST
%token HEAD TAIL CONS SHOW TYPE

%nonassoc IFX
%nonassoc ELSE

%left NOT
%left AND OR
%left GE LE EQ NE APPEND '>' '<'
%left '+' '-'
%left '*' '/' MOD

%nonassoc UMINUS

%type <nPtr> program stmts stmt expr assign params list identifier end

%%

program :
stmts                                                 { temp = mkcons(PROG, 1, $1); }
;

stmts:
stmt                                                  { $$ = $1; }
| stmts stmt                                          { $$ = mkcons(';', 2, $1, $2); }
;

stmt:
DEF assign                                            { $$ = $2; }
| IF expr THEN stmts %prec IFX end                    { $$ = mkcons(IF, 3, $2, $4, $5); }
| IF expr THEN stmts ELSE stmts end                   { $$ = mkcons(IF, 4, $2, $4, $6, $7); }
| USE identifier                                      { $$ = mkcons(USE, 1, $2); }
| expr                                                { $$ = $1; }
;

end :
END                                                   { $$ = mkcons(END, 0); }
;

expr:
INTEGER                                               { $$ = mkint($1); }
| FLOAT                                               { $$ = mkfloat($1); }
| CHAR                                                { $$ = mkchar($1); }
| TRUE                                                { $$ = mkbool(1); }
| FALSE                                               { $$ = mkbool(0); }
| STRING                                              { $$ = mkstr($1); }
| NIL                                                 { $$ = mknil(); }
| identifier                                          { $$ = $1; }
| LAMBDA '(' params ')' stmts END                     { $$ = mkcons(LAMBDA, 2, $3, $5); }
| identifier '(' params ')'                           { $$ = mkcons(FUNCALL, 2, $1, $3); }
| identifier ':' identifier '(' params ')'            { $$ = mkcons(MODULECALL, 3, $1, $3, $5); }
| HEAD '(' expr ')'                                   { $$ = mkcons(HEAD, 1, $3); }
| TAIL '(' expr ')'                                   { $$ = mkcons(TAIL, 1, $3); }
| CONS '(' expr ',' expr ')'                          { $$ = mkcons(CONS, 2, $3, $5); }
| SHOW '(' expr ')'                                   { $$ = mkcons(SHOW, 1, $3); }
| TYPE '(' expr ')'                                   { $$ = mkcons(TYPE, 1, $3); }
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

assign:
identifier '=' expr                                   { $$ = mkcons(ASSIGN, 2, $1, $3); }
| assign ',' assign                                   { $$ = mkcons(';', 2, $1, $3); }
;

identifier:
SYMBOL                                                { $$ = mksym($1); }
;

params:
expr                                                  { $$ = mkcons(',', 1, $1); }
| expr ',' params                                     { $$ = mkcons(',', 2, $1, $3); }
| /* empty lambda list */                             { $$ = mkcons(',', 0); }
;

list:
expr                                                  { $$ = mkcons(LIST, 1, $1); }
| expr ',' list                                       { $$ = mkcons(LIST, 2, $1, $3); }
| /* empty list */                                    { $$ = mkcons(LIST, 0); }
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
