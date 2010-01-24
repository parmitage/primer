%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"

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

%token <sval> SYMBOL
%token <ival> INTEGER
%token <fval> FLOAT
%token PROG DEF LAMBDA IF ELSE FUNCALL
%token GE LE NE EQ AND OR MOD APPEND TRUE FALSE NIL END LIST
%token HEAD TAIL CONS SHOW

%nonassoc IFX
%nonassoc ELSE

%left AND OR
%left GE LE EQ NE MOD APPEND '>' '<'
%left '+' '-'
%left '*' '/'

%type <nPtr> program stmts stmt expr params list identifier end

%%

program :
	stmts											{ ast = opr(PROG, 1, $1); }
	;

stmts:
	stmt											{ $$ = $1; }
	| stmts stmt									{ $$ = opr(';', 2, $1, $2); }
	;

stmt:
	DEF identifier '=' expr							{ $$ = opr(DEF, 2, $2, $4); }
	| IF '(' expr ')' stmts %prec IFX end			{ $$ = opr(IF, 3, $3, $5, $6); }
	| IF '(' expr ')' stmts ELSE stmts end			{ $$ = opr(IF, 4, $3, $5, $7, $8); }
	| expr											{ $$ = $1; }
	;

end :
	END												{ $$ = opr(END, 0); }
	;

expr:
	INTEGER											{ $$ = con($1); }
	| FLOAT                                         { $$ = fpval($1); }
	| TRUE                                          { $$ = boolval(1); }
	| FALSE                                         { $$ = boolval(0); }
	| NIL											{ $$ = nil(); }
	| identifier									{ $$ = $1; }
	| LAMBDA '(' params ')' stmts END				{ $$ = opr(LAMBDA, 2, $3, $5); }
	| identifier '(' params ')'						{ $$ = opr(FUNCALL, 2, $1, $3); }
	| HEAD '(' expr ')'								{ $$ = opr(HEAD, 1, $3); }
	| TAIL '(' expr ')'								{ $$ = opr(TAIL, 1, $3); }
	| CONS '(' expr ',' expr ')'					{ $$ = opr(CONS, 2, $3, $5); }
	| SHOW '(' expr ')'								{ $$ = opr(SHOW, 1, $3); }
	| expr '+' expr									{ $$ = opr('+', 2, $1, $3); }
	| expr '-' expr									{ $$ = opr('-', 2, $1, $3); }
	| expr '*' expr									{ $$ = opr('*', 2, $1, $3); }
	| expr '/' expr									{ $$ = opr('/', 2, $1, $3); }
	| expr '<' expr									{ $$ = opr('<', 2, $1, $3); }
	| expr '>' expr									{ $$ = opr('>', 2, $1, $3); }
	| expr GE expr									{ $$ = opr(GE, 2, $1, $3); }
	| expr LE expr									{ $$ = opr(LE, 2, $1, $3); }
	| expr NE expr									{ $$ = opr(NE, 2, $1, $3); }
	| expr EQ expr									{ $$ = opr(EQ, 2, $1, $3); }
	| expr AND expr									{ $$ = opr(AND, 2, $1, $3); }
	| expr OR expr									{ $$ = opr(OR, 2, $1, $3); }
	| expr MOD expr									{ $$ = opr(MOD, 2, $1, $3); }
	| expr APPEND expr								{ $$ = opr(APPEND, 2, $1, $3); }
	| '[' list ']'									{ $$ = $2; }
	;

identifier:
	SYMBOL											{ $$ = sym($1); }
	;

params:
	expr											{ $$ = opr(',', 1, $1); }
	| expr ',' params								{ $$ = opr(',', 2, $1, $3); }
	| /* empty lambda list */						{ $$ = opr(',', 0); }
	;

list:
	expr											{ $$ = opr(LIST, 1, $1); }
	| expr ',' list									{ $$ = opr(LIST, 2, $1, $3); }
	| /* empty list */								{ $$ = opr(LIST, 0); }
	;

%%

void yyerror(char *s)
{
	char* errmsg = malloc(200);
	errmsg[0] = '\0';
	logerr(s, lineno);
}

void parse(char* filename)
{
	if ((yyin = fopen(filename, "r")) == NULL)
	{
		printf("Error reading input file\n");
	}
	
	yyparse();
}
