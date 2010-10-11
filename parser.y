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
%token GE LE NE EQ NOT AND OR MOD APPEND TRUE FALSE END LIST
%token SHOW TYPE LENGTH NTH CONS WHERE RANGE

%nonassoc ELSE
%left PAREN
%left NOT
%left AND OR APPEND
%left GE LE EQ NE RANGE '>' '<'
%left '+' '-'
%left '*' '/' MOD NTH
%right CONS
%nonassoc UMINUS

%type <nPtr> program stmts stmt expr list identifier

%%

program :
stmts                                                 { temp = mkpair(PROG, 1, $1); }
;

stmts:
stmt                                                  { $$ = $1; }
| stmt stmts                                          { $$ = mkpair(';', 2, $1, $2); }
;

stmt:
expr '=' expr                                         { $$ = mkpair(DEF, 2, $1, $3); }
| expr                                                { $$ = $1; }
;

expr:
'(' expr ')'                                          { $$ = $2; }
| INTEGER                                             { $$ = mkint($1); }
| FLOAT                                               { $$ = mkfloat($1); }
| CHAR                                                { $$ = mkchar($1); }
| TRUE                                                { $$ = NODE_BOOL_TRUE; }
| FALSE                                               { $$ = NODE_BOOL_FALSE; }
| STRING                                              { $$ = mkstr($1); }
| identifier                                          { $$ = $1; }
| LAMBDA '(' list ')' expr END                        { $$ = mkpair(LAMBDA, 2, $3, $5); }
| LAMBDA '(' list ')' expr WHERE stmts END            { $$ = mkpair(LAMBDA, 3, $3, $5, $7); }
| identifier '(' list ')'                             { $$ = mkpair(APPLY, 2, $1, $3); }
| IF expr THEN expr ELSE expr                         { $$ = mkpair(IF, 3, $2, $4, $6); }
| expr CONS expr                                      { $$ = mkpair(CONS, 2, $1, $3); }
| TYPE '(' expr ')'                                   { $$ = mkoperator(type, $3, NULL); }
| SHOW '(' expr ')'                                   { $$ = mkoperator(show, $3, NULL); }
| LENGTH '(' expr ')'                                 { $$ = mkoperator(len, $3, NULL); }
| expr '+' expr                                       { $$ = mkoperator(add, $1, $3); }
| expr '-' expr                                       { $$ = mkoperator(sub, $1, $3); }
| expr '*' expr                                       { $$ = mkoperator(mul, $1, $3); }
| expr '/' expr                                       { $$ = mkoperator(dvd, $1, $3); }
| expr '<' expr                                       { $$ = mkoperator(lt, $1, $3); }
| expr '>' expr                                       { $$ = mkoperator(gt, $1, $3); }
| expr GE expr                                        { $$ = mkoperator(gte, $1, $3); }
| expr LE expr                                        { $$ = mkoperator(lte, $1, $3); }
| expr NE expr                                        { $$ = mkoperator(neq, $1, $3); }
| expr EQ expr                                        { $$ = mkoperator(eq, $1, $3); }
| expr AND expr                                       { $$ = mkoperator(and, $1, $3); }
| expr OR expr                                        { $$ = mkoperator(or, $1, $3); }
| NOT expr                                            { $$ = mkoperator(not, $2, NULL); }
| expr MOD expr                                       { $$ = mkoperator(mod, $1, $3); }
| expr APPEND expr                                    { $$ = mkoperator(append, $1, $3); }
| expr RANGE expr                                     { $$ = mkoperator(range, $1, $3); }
| expr NTH expr                                       { $$ = mkoperator(nth, $1, $3); }
| '-' expr %prec UMINUS                               { $$ = mkoperator(neg, $2, NULL); }
| '[' list ']'                                        { $$ = $2; }
;

identifier:
SYMBOL                                                { $$ = mksym($1); }
;

list:
expr                                                  { $$ = mkpair(LIST, 1, $1); }
| expr ',' list                                       { $$ = mkpair(LIST, 2, $1, $3); }
| /* empty list */                                    { $$ = mkpair(LIST, 0); }
;

%%

void yyerror(char *s)
{
   char* errmsg = malloc(200);
   errmsg[0] = '\0';
   error(s);
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
