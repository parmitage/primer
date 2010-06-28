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
%token PROG DEF LAMBDA IF THEN ELSE ELIF COND APPLY
%token GE LE NE EQ NOT AND OR MOD APPEND TRUE FALSE NIL END LIST
%token SHOW TYPE LENGTH NTH MATCH WHERE PAREN RANGE

%nonassoc ELSE
%left PAREN
%left NOT
%left AND OR APPEND
%left GE LE EQ NE RANGE '>' '<'
%left '+' '-'
%left '*' '/' MOD NTH
%right MATCH
%nonassoc UMINUS

%type <nPtr> program stmts stmt expr list identifier

%%

program :
stmts                                                 { temp = mkcons(PROG, 1, $1); }
;

stmts:
stmt                                                  { $$ = $1; }
| stmts stmt                                          { $$ = mkcons(';', 2, $1, $2); }
;

stmt:
identifier '=' expr                                   { $$ = mkcons(DEF, 2, $1, $3); }
| expr                                                { $$ = $1; }
;

expr:
'(' expr ')'                                          { $$ = mkcons(PAREN, 1, $2); }
| INTEGER                                             { $$ = mkint($1); }
| FLOAT                                               { $$ = mkfloat($1); }
| CHAR                                                { $$ = mkchar($1); }
| TRUE                                                { $$ = NODE_BOOL_TRUE; }
| FALSE                                               { $$ = NODE_BOOL_FALSE; }
| STRING                                              { $$ = mkstr($1); }
| NIL                                                 { $$ = NODE_NIL; }
| identifier                                          { $$ = $1; }
| LAMBDA '(' list ')' expr END                        { $$ = mkcons(LAMBDA, 2, $3, $5); }
| LAMBDA '(' list ')' expr WHERE stmts END            { $$ = mkcons(LAMBDA, 3, $3, $5, $7); }
| identifier '(' list ')'                             { $$ = mkcons(APPLY, 2, $1, $3); }
| IF expr THEN expr ELSE expr                         { $$ = mkcons(IF, 3, $2, $4, $6); }
| SHOW '(' expr ')'                                   { $$ = mkcons(SHOW, 1, $3); }
| TYPE '(' expr ')'                                   { $$ = mkcons(TYPE, 1, $3); }
| LENGTH '(' expr ')'                                 { $$ = mkcons(LENGTH, 1, $3); }
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
| expr RANGE expr                                     { $$ = mkcons(RANGE, 2, $1, $3); }
| expr NTH expr                                       { $$ = mkcons(NTH, 2, $1, $3); }
| expr MATCH expr                                     { $$ = mkcons(MATCH, 2, $1, $3); }
| NOT expr                                            { $$ = mkcons(NOT, 1, $2); }
| '-' expr %prec UMINUS                               { $$ = mkcons('-', 1, $2); }
| '[' list ']'                                        { $$ = $2; }
;

identifier:
SYMBOL                                                { $$ = mksym($1); }
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
