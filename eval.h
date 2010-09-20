#ifndef __EVAL_H__
#define __EVAL_H__

#include "main.h"

#define MAX_SYMBOLS 1000

typedef enum {
   t_any,
   t_num,
   t_int,
   t_float,
   t_bool,
   t_symbol,
   t_char,
   t_pair,
   t_list,
   t_tuple,
   t_closure
} t_type;

typedef struct pair {
   int oper;
   int nops;
   struct env *env;
   struct node *op[3];
} pair;

typedef struct node {
   t_type type;
   int lineno;
   int rc;
   union {
      int ival;
      float fval;
      char* sval;
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
env *global, *tco_env;
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
void envext(env *e, binding *binding);
binding *envlookup(env *e, symbol sym);
symbol intern(char *string);
char *symname(symbol s);

/* primitive operators */
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

/* misc utils */
node *loadlib(char *name);
void error(char *msg);

#endif
