#ifndef __EVAL_H__
#define __EVAL_H__

#include "main.h"

#define MAX_SYMBOLS 1000

typedef enum {
   t_int,
   t_float,
   t_bool,
   t_symbol,
   t_char,
   t_pair,
   t_closure,
   t_operator
} t_type;

typedef struct pair {
   int oper;
   int nops;
   struct node *op[3];
} pair;

typedef struct closure {
   struct node *args;
   struct node *body;
   struct node *where;
   struct env *env;
} closure;

typedef struct operator {
   struct node * (*primitive) (struct node *);
   struct node *arg1;
   struct node *arg2;
} operator;

typedef struct node {
   t_type type;
   int lineno;
   int rc;
   union {
      int ival;
      float fval;
      char* sval;
      closure *fn;
      operator *op;
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
env *top, *tco_env;
int lineno;
symbol wildcard;

static long cnt_alloc = 0;
static long cnt_free = 0;
static long cnt_inc = 0;
static long cnt_dec = 0;

/* abstract syntax tree */
void parse(char *filename);
node *parsel(char *filename);

/* constructors */
node *mkpair(int oper, int nops, ...);
node *mksym(char *s);
node *mkprimitive(struct node * (*primitive) (struct node *));
node *mkoperator(struct node * (*op) (struct node *), node *arg1, node *arg2);
node *mklambda(node *params, node *body, node *where, env *e);
node *mkint(int value);
node *mkfloat(float value);
node *mkbool(int value);
node *mkchar(char value);
node *mkstr(char *value);
node *strtonode(char *value);

/* memory management */
struct node *prialloc();
void incref(node *n);
void decref(node *n);

/* evaluation */
node *eval(node *p, env *e);
bool istailrecur(node *expr, symbol s);
void bindarg(node *args, node *params, env *fnenv, env *argenv);
void bind(node *lhs, node *rhs, env *env);
void bindp(node *args, node *list, env *fnenv);
binding *bindnew(symbol name, node* node);
env *envnew(env* enclosing);
env *envdel(env* e);
void extend(env *e, binding *binding);
binding *envlookup(env *e, symbol sym);
symbol intern(char *string);
char *symname(symbol s);

/* special forms */
void pprint(node *node);
bool empty(node *list);
node *list_eq(node *l1, node *l2);
node *cons(node *atom, node *list);

/* primitive operators */
node *car(node *node);
node *cdr(node *node);
node *len(node *node);
node *nth(node *args);
node *add(node *args);
node *sub(node *args);
node *neg(node *args);
node *mul(node *args);
node *dvd(node *args);
node *lt(node *args);
node *gt(node *args);
node *lte(node *args);
node *gte(node *args);
node *eq(node *args);
node *neq(node *args);
node *and(node *args);
node *or(node *args);
node *not(node *args);
node *mod(node *args);
node *type(node *args);
node *append(node *args);
node *range(node *args);
node *show(node *args);

/* misc utils */
node *loadlib(char *name);
void error(char *msg);

#endif
