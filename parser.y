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
%token PROG LET VAL DEF DEFINED IN LAMBDA IF THEN ELSE MATCH WITH ANY APPLY
%token GE LE NE EQ NOT AND OR MOD APPEND TRUE FALSE LIST
%token HEAD TAIL SHOW READ RND TYPE IS AS LENGTH AT CONS RANGE USING PRAGMA
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
| LET let_block IN expr               { $$ = mkast(t_let, $2, $4, NULL); }
| VAL identifier DEF expr             { $$ = mkast(t_val, $2, $4, NULL);  }
| LAMBDA args DEFINED expr            { $$ = mkast(t_lambda, $2, $4, NULL); }
| identifier LPAREN list RPAREN       { $$ = mkast(t_apply, $1, $3, NULL); }
| IF expr THEN expr ELSE expr         { $$ = mkast(t_cond, $2, $4, $6); }
| expr CONS expr                      { $$ = mkast(t_cons, $1, $3, NULL); }
| SHOW LPAREN expr RPAREN             { $$ = mkoperator(show, $3); }
| READ LPAREN RPAREN                  { $$ = mkoperator(reads, NULL); }
| LENGTH LPAREN expr RPAREN           { $$ = mkoperator(len, $3); }
| HEAD LPAREN expr RPAREN             { $$ = mkast(t_car, $3, NULL, NULL); }
| TAIL LPAREN expr RPAREN             { $$ = mkast(t_cdr, $3, NULL, NULL); }
| USING identifier                    { $$ = mkast(t_using, $2, NULL, NULL); }
| PRAGMA identifier                   { pragma($2); }
| RND LPAREN expr RPAREN              { $$ = mkoperator(rnd, $3); }
| expr '+' expr                       { $$ = mkbinoperator(add, $1, $3); }
| expr '-' expr                       { $$ = mkbinoperator(sub, $1, $3); }
| expr '*' expr                       { $$ = mkbinoperator(mul, $1, $3); }
| expr '/' expr                       { $$ = mkbinoperator(dvd, $1, $3); }
| expr '<' expr                       { $$ = mkbinoperator(lt, $1, $3); }
| expr '>' expr                       { $$ = mkbinoperator(gt, $1, $3); }
| expr GE expr                        { $$ = mkbinoperator(gte, $1, $3); }
| expr LE expr                        { $$ = mkbinoperator(lte, $1, $3); }
| expr NE expr                        { $$ = mkbinoperator(neq, $1, $3); }
| expr EQ expr                        { $$ = mkbinoperator(eq, $1, $3); }
| expr AND expr                       { $$ = mkbinoperator(and, $1, $3); }
| expr OR expr                        { $$ = mkbinoperator(or, $1, $3); }
| expr B_AND expr                     { $$ = mkbinoperator(b_and, $1, $3); }
| expr B_OR expr                      { $$ = mkbinoperator(b_or, $1, $3); }
| expr B_XOR expr                     { $$ = mkbinoperator(b_xor, $1, $3); }
| B_NOT expr                          { $$ = mkoperator(b_not, $2); }
| expr B_LSHIFT expr                  { $$ = mkbinoperator(b_lshift, $1, $3); }
| expr B_RSHIFT expr                  { $$ = mkbinoperator(b_rshift, $1, $3); }
| NOT expr                            { $$ = mkoperator(not, $2); }
| expr MOD expr                       { $$ = mkbinoperator(mod, $1, $3); }
| expr APPEND expr                    { $$ = mkbinoperator(append, $1, $3); }
| expr RANGE expr                     { $$ = mkbinoperator(range, $1, $3); }
| expr AT expr                        { $$ = mkbinoperator(at, $1, $3); }
| expr AS expr                        { $$ = mkbinoperator(as, $1, $3); }
| expr IS expr                        { $$ = mkbinoperator(is, $1, $3); }
| '-' expr %prec UMINUS               { $$ = mkoperator(neg, $2); }
| LSQUARE list RSQUARE                { $$ = $2; }
| MATCH list pattern_block            { $$ = mkast(t_match, $2, $3, NULL); }
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
