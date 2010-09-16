#ifndef __EVAL_H__
#define __EVAL_H__

#include "main.h"
#include "types.h"

#define MAX_SYMBOLS 1000

typedef struct pair {
   int oper;
   int nops;
   struct env *env;
   struct node *op[3];
} pair;

typedef struct tuple {
   int count;
   struct node *n[MAX_TUPLES]; 
} tuple;

typedef struct node {
   t_type type;
   int lineno;
   int rc;
   union {
      int ival;
      float fval;
      char* sval;
      tuple tuple;
      pair opr;
   };
} node;

typedef int symbol;
char *symtab[MAX_SYMBOLS];

typedef struct binding {
   symbol sym;
   struct node *node;
   struct binding *prev;
} binding;

typedef struct env {
   struct env *parent;
   binding *bind;
} env;

node *NODE_BOOL_TRUE, *NODE_BOOL_FALSE, *temp, *ast;
env *global;
int lineno;
symbol wildcard;

void parse(char *filename);
node *parsel(char *filename);

node *mkpair(int oper, int nops, ...);
node *mksym(char *s);
node *mklambda(node *params, node *body, node *where, env *e);
node *mkint(int value);
node *mkfloat(float value);
node *mkbool(int value);
node *mkchar(char value);
node *mkstr(char *value);
node *mktuple();
node *list2tuple(node *list);
node *strtonode(char *value);

struct node *prialloc();
void incref(node *n);
void decref(node *n);

node *eval(node *p, env *e);
bool istailrecur(node *expr, symbol s);

void bind(node *args, node *params, env *fnenv, env *argenv);
void bindt(node *arg, node *tuple, env *fnenv);
void bindp(node *args, node *list, env *fnenv);
binding *bindnew(symbol name, node* node);
env *envnew(env* enclosing);
env *envdel(env* e);
void envext(env *e, binding *binding);
binding *envlookup(env *e, symbol sym);

symbol intern(char *string);
char *symname(symbol s);

node *car(node *node);
node *cdr(node *node);
int length(node *node);
void display(node *node);
void pprint(node *node);
bool empty(node *list);

node *add(node *x, node *y);
node *sub(node *x, node *y);
node *mul(node *x, node *y);
node *dvd(node *x, node *y);
node *lt(node *x, node *y);
node *gt(node *x, node *y);
node *lte(node *x, node *y);
node *gte(node *x, node *y);
node *eq(node *x, node *y);
node *neq(node *x, node *y);
node *list_eq(node *l1, node *l2);
node *and(node *x, node *y);
node *or(node *x, node *y);
node *not(node *node);
node *mod(node *x, node *y);
node *cons(node *atom, node *list);
node *append(node *list1, node *list2);
node *range(node *from, node *to);

node *loadlib(char *name);
void error(char *msg);

#endif
