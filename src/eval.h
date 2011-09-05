#ifndef __EVAL_H__
#define __EVAL_H__

#include "main.h"

char *symtab[MAX_SYMBOLS];

node *NODE_BOOL_TRUE, *NODE_BOOL_FALSE, *NODE_ANY, *temp;
env *top, *cenv, *tco_env;

char *libcache[50];
int lastlib;
bool pragma_gc_disable;

/* abstract syntax tree */
node *parse(char *filename);

/* constructors */
node *mkpair(t_type type, node *car, node* cdr);
node *mksym(char *s);
node *mkprimitive(struct node * (*primitive) (struct node *));
node *mkoperator(struct node * (*op) (struct node *), node *arg1);
node *mkbinoperator(struct node * (*binop) (struct node *, struct node *), node *arg1, node *arg2);
node *mklambda(node *params, node *body, node *where);
node *mkclosure(node *params, node *body, env *e);
node *mkint(int value);
node *mkint_literal(int value);
node *mkfloat(float value);
node *mkbool(int value);
node *mkchar(char value);
node *mkstr(char *value);
node *mkapply(node *fn, node *args);
node *mkcond(node *predicate, node *consequent, node *alternate);
node *mkseq(node *this, node *next);
node *mkast(t_type type, node *n1, node *n2, node *n3);

/* primer 'string' handling */ 
node *str_to_node(char *value);
char *node_to_str(node *node);

/* memory management */
struct node *prialloc();

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
symbol intern(char *string);
char *symname(symbol s);

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
node *reads(node *args);

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
