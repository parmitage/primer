%{
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "pri.h"

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
%token PROG LET VAL DEF DEFINED IN LAMBDA IF THEN ELSE MATCH WITH ANY APPLY
%token GE LE NE EQ NOT AND OR MOD APPEND TRUE FALSE LIST
%token HEAD TAIL SHOW READ RND TYPE IS AS LENGTH AT CONS RANGE USING
%token B_AND B_OR B_XOR B_NOT B_LSHIFT B_RSHIFT
%token SEMICOLON LPAREN RPAREN LSQUARE RSQUARE

%nonassoc ELSE
%left IN
%left LET DEF DEFINED LPAREN RPAREN NOT B_NOT
%left AND OR APPEND MATCH WITH THEN
%left LT GT GE LE EQ NE RANGE '>' '<'
%left '+' '-'
%left '*' '/' MOD AT AS IS
%left B_AND B_OR B_XOR B_LSHIFT B_RSHIFT USING
%right CONS
%nonassoc UMINUS

%type <nPtr> program exprs expr pattern_block pattern list args identifier let_block def

%%

program :
exprs                                 { temp = $1; }
;

exprs:
expr SEMICOLON                        { $$ = $1; }
| expr SEMICOLON exprs                { $$ = mkast(t_seq, $1, $3, NULL); }
;

expr:
LPAREN expr RPAREN                    { $$ = $2; }
| INTEGER                             { $$ = mkint($1); }
| FLOAT                               { $$ = mkfloat($1); }
| CHAR                                { $$ = mkchar($1); }
| TRUE                                { $$ = NODE_BOOL_TRUE; }
| FALSE                               { $$ = NODE_BOOL_FALSE; }
| STRING                              { $$ = mkstr($1); }
| identifier                          { $$ = $1; }
| ANY                                 { $$ = NODE_ANY; }
| LSQUARE list RSQUARE                { $$ = $2; }
| LET let_block IN expr               { $$ = mkast(t_let, $2, $4, NULL); }
| VAL identifier DEF expr             { $$ = mkast(t_val, $2, $4, NULL);  }
| LAMBDA args DEFINED expr            { $$ = mkast(t_lambda, $2, $4, NULL); }
| identifier LPAREN list RPAREN       { $$ = mkast(t_apply, $1, $3, NULL); }
| IF expr THEN expr ELSE expr         { $$ = mkast(t_cond, $2, $4, $6); }
| expr CONS expr                      { $$ = mkast(t_cons, $1, $3, NULL); }
| SHOW LPAREN expr RPAREN             { $$ = mkast(t_show, $3, NULL, NULL); }
| READ LPAREN RPAREN                  { $$ = mkast(t_reads, NULL, NULL, NULL); }
| LENGTH LPAREN expr RPAREN           { $$ = mkast(t_len, $3, NULL, NULL); }
| HEAD LPAREN expr RPAREN             { $$ = mkast(t_car, $3, NULL, NULL); }
| TAIL LPAREN expr RPAREN             { $$ = mkast(t_cdr, $3, NULL, NULL); }
| USING identifier                    { $$ = mkast(t_using, $2, NULL, NULL); }
| B_NOT expr                          { $$ = mkast(t_bnot, $2, NULL, NULL); }
| RND LPAREN expr RPAREN              { $$ = mkast(t_rnd, $3, NULL, NULL); }
| NOT expr                            { $$ = mkast(t_not, $2, NULL, NULL); }
| '-' expr %prec UMINUS               { $$ = mkast(t_neg, $2, NULL, NULL); }
| MATCH list pattern_block            { $$ = mkast(t_match, $2, $3, NULL); }
| expr '+' expr                       { $$ = mkast(t_add, $1, $3, NULL); }
| expr '-' expr                       { $$ = mkast(t_sub, $1, $3, NULL); }
| expr '*' expr                       { $$ = mkast(t_mul, $1, $3, NULL); }
| expr '/' expr                       { $$ = mkast(t_dvd, $1, $3, NULL); }
| expr '<' expr                       { $$ = mkast(t_lt, $1, $3, NULL); }
| expr '>' expr                       { $$ = mkast(t_gt, $1, $3, NULL); }
| expr GE expr                        { $$ = mkast(t_gte, $1, $3, NULL); }
| expr LE expr                        { $$ = mkast(t_lte, $1, $3, NULL); }
| expr NE expr                        { $$ = mkast(t_neq, $1, $3, NULL); }
| expr EQ expr                        { $$ = mkast(t_eq, $1, $3, NULL); }
| expr AND expr                       { $$ = mkast(t_and, $1, $3, NULL); }
| expr OR expr                        { $$ = mkast(t_or, $1, $3, NULL); }
| expr B_AND expr                     { $$ = mkast(t_b_and, $1, $3, NULL); }
| expr B_OR expr                      { $$ = mkast(t_b_or, $1, $3, NULL); }
| expr B_XOR expr                     { $$ = mkast(t_b_xor, $1, $3, NULL); }
| expr B_LSHIFT expr                  { $$ = mkast(t_b_lshift, $1, $3, NULL); }
| expr B_RSHIFT expr                  { $$ = mkast(t_b_rshift, $1, $3, NULL); }
| expr MOD expr                       { $$ = mkast(t_mod, $1, $3, NULL); }
| expr APPEND expr                    { $$ = mkast(t_append, $1, $3, NULL); }
| expr RANGE expr                     { $$ = mkast(t_range, $1, $3, NULL); }
| expr AT expr                        { $$ = mkast(t_at, $1, $3, NULL); }
| expr AS expr                        { $$ = mkast(t_as, $1, $3, NULL); }
| expr IS expr                        { $$ = mkast(t_is, $1, $3, NULL); }
;

identifier:
SYMBOL                                { $$ = mksym($1); }
;

let_block:
def                                   { $$ = mkast(t_seq, $1, NULL, NULL); }
| def let_block                       { $$ = mkast(t_seq, $1, $2, NULL); }
;

def:
identifier DEF expr                   { $$ = mkast(t_val, $1, $3, NULL);  }
;

args:
identifier                            { $$ = mkpair(t_pair, $1, NULL); }
| identifier args                     { $$ = mkpair(t_pair, $1, $2); }
;

list:
expr                                  { $$ = mkpair(t_pair, $1, NULL); }
| expr ',' list                       { $$ = mkpair(t_pair, $1, $3); }
| /* empty list */                    { $$ = mkpair(t_pair, NULL, NULL); }
;

pattern_block:
pattern                               { $$ = mkpair(t_pair, $1, NULL); }
| pattern pattern_block               { $$ = mkpair(t_pair, $1, $2); }
;

pattern:
WITH list THEN expr                   { $$ = mkpair(t_pattern, $2, $4); }
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
      error("unable to read input file");
   }
   
   yyparse();
   return temp;
}
