#ifndef __EVAL_H__
#define __EVAL_H__

#include "main.h"

env *top, *cenv, *tco_env;
char *libcache[50];
int lastlib;
bool pragma_gc_disable;

/* evaluation */
node *eval(node *p, env *e);
node *evlis(node *list, env *env);
bool istailrecur(node *expr, symbol s);
void bind(node *args, node *params, env *fnenv, env *argenv);
binding *bindnew(symbol name, node* node);
env *envnew(env* enclosing);
env *envdel(env* e);
void envprint(env *e, bool depth);
void extend(env *e, binding *binding);
binding *envlookup(env *e, symbol sym);
node *mkclosure(node *args, node *body, env *env);

/* special forms */
void pprint(node *node);
node *list_eq(node *l1, node *l2);
node *cons(node *atom, node *list);

/* list operators */
node *car(node *node);
node *cdr(node *node);
node *len(node *node);
node *at(node *x, node *y);
node *append(node *x, node *y);
node *range(node *x, node *y);

/* math operators */
node *add(node *x, node *y);
node *sub(node *x, node *y);
node *neg(node *x);
node *mul(node *x, node *y);
node *dvd(node *x, node *y);
node *mod(node *x, node *y);

/* logic operators */
node *lt(node *x, node *y);
node *gt(node *x, node *y);
node *lte(node *x, node *y);
node *gte(node *x, node *y);
node *eq(node *x, node *y);
node *neq(node *x, node *y);
node *and(node *x, node *y);
node *or(node *x, node *y);
node *not(node *x);

/* bitwise operators */
node *b_or(node *x, node *y);
node *b_and(node *x, node *y);
node *b_xor(node *x, node *y);
node *b_not(node *x);
node *b_lshift(node *x, node *y);
node *b_rshift(node *x, node *y);

/* type operators */
node *is(node *exp, node *type);
node *as(node *x, node *y);

/* primitive functions */
node *rnd(node *node);
node *show(node *args);
node *reads();

/* pre-processing */
void using(node *args);
void pragma(node *args);

/* utils */
void init_stage1();
void init_stage2();
node *loadlib(char *name);
void error(char* fmt, ...);
bool fexists(const char *path);
bool is_any_pattern(node *n);

#endif
