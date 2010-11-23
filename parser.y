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
%token HEAD TAIL SHOW TYPE IS AS LENGTH AT CONS WHERE RANGE

%nonassoc ELSE
%left PAREN
%left NOT
%left AND OR APPEND
%left GE LE EQ NE RANGE '>' '<'
%left '+' '-'
%left '*' '/' MOD AT AS IS
%right CONS
%nonassoc UMINUS

%type <nPtr> program stmts stmt expr list identifier

%%

program :
stmts                                                 { temp = $1; }
;

stmts:
stmt                                                  { $$ = $1; }
| stmt stmts                                          { $$ = mkast(t_seq, $1, $2, NULL); }
;

stmt:
expr ':' expr                                         { $$ = mkast(t_def, $1, $3, NULL); }
| expr                                                { $$ = $1; }
;

expr:
'(' expr ')'                                          { $$ = $2; }
| INTEGER                                             { $$ = literally(mkint($1)); }
| FLOAT                                               { $$ = literally(mkfloat($1)); }
| CHAR                                                { $$ = literally(mkchar($1)); }
| TRUE                                                { $$ = NODE_BOOL_TRUE; }
| FALSE                                               { $$ = NODE_BOOL_FALSE; }
| STRING                                              { $$ = mkstr($1); }
| identifier                                          { $$ = $1; }
| LAMBDA '(' list ')' expr END                        { $$ = mkast(t_lambda, $3, $5, NULL); }
| LAMBDA '(' list ')' expr WHERE stmts END            { $$ = mkast(t_lambda, $3, $5, $7); }
| identifier '(' list ')'                             { $$ = mkast(t_apply, $1, $3, NULL); }
| IF expr THEN expr ELSE expr                         { $$ = mkast(t_cond, $2, $4, $6); }
| expr CONS expr                                      { $$ = mkast(t_cons, $1, $3, NULL); }
| SHOW '(' expr ')'                                   { $$ = mkoperator(show, $3); }
| LENGTH '(' expr ')'                                 { $$ = mkoperator(len, $3); }
| HEAD '(' expr ')'                                   { $$ = mkast(t_car, $3, NULL, NULL); }
| TAIL '(' expr ')'                                   { $$ = mkast(t_cdr, $3, NULL, NULL); }
| expr '+' expr                                       { $$ = mkbinoperator(add, $1, $3); }
| expr '-' expr                                       { $$ = mkbinoperator(sub, $1, $3); }
| expr '*' expr                                       { $$ = mkbinoperator(mul, $1, $3); }
| expr '/' expr                                       { $$ = mkbinoperator(dvd, $1, $3); }
| expr '<' expr                                       { $$ = mkbinoperator(lt, $1, $3); }
| expr '>' expr                                       { $$ = mkbinoperator(gt, $1, $3); }
| expr GE expr                                        { $$ = mkbinoperator(gte, $1, $3); }
| expr LE expr                                        { $$ = mkbinoperator(lte, $1, $3); }
| expr NE expr                                        { $$ = mkbinoperator(neq, $1, $3); }
| expr EQ expr                                        { $$ = mkbinoperator(eq, $1, $3); }
| expr AND expr                                       { $$ = mkbinoperator(and, $1, $3); }
| expr OR expr                                        { $$ = mkbinoperator(or, $1, $3); }
| NOT expr                                            { $$ = mkoperator(not, $2); }
| expr MOD expr                                       { $$ = mkbinoperator(mod, $1, $3); }
| expr APPEND expr                                    { $$ = mkbinoperator(append, $1, $3); }
| expr RANGE expr                                     { $$ = mkbinoperator(range, $1, $3); }
| expr AT expr                                        { $$ = mkbinoperator(at, $1, $3); }
| expr AS expr                                        { $$ = mkbinoperator(as, $1, $3); }
| expr IS expr                                        { $$ = mkbinoperator(is, $1, $3); }
| '-' expr %prec UMINUS                               { $$ = mkoperator(neg, $2); }
| '[' list ']'                                        { $$ = $2; }
;

identifier:
SYMBOL                                                { $$ = mksym($1); }
;

list:
expr                                                  { $$ = mkpair(t_pair, $1, NULL); }
| expr ',' list                                       { $$ = mkpair(t_pair, $1, $3); }
| /* empty list */                                    { $$ = mkpair(t_pair, NULL, NULL); }
;

%%

void yyerror(char *s)
{
   char* errmsg = malloc(200);
   errmsg[0] = '\0';
   error(s);
}

node* parse(char* filename)
{
   if ((yyin = fopen(filename, "r")) == NULL)
   {
      printf("Error reading input file\n");
   }
   
   yyparse();
   return temp;
}
