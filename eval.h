#ifndef __EVAL_H__
#define __EVAL_H__

#include "main.h"

#define MAX_SYMBOLS 1000

#define CAR(p) (p != NULL && p->pair->car != NULL ? p->pair->car : NULL)
#define CDR(p) (p != NULL && p->pair->cdr != NULL ? p->pair->cdr : NULL)
#define EMPTY(p) (p == NULL || p->pair->car == NULL)

#define EXTRACT_NUMBER(x) (x->type == t_float ? x->fval : x->ival)
#define NUMERIC_RETURN_TYPE(x, y) (x->type == t_float || y->type == t_float ? t_float : t_int)
#define DIVIDE_RETURN_TYPE(x, y) (x->type == t_float || y->type == t_float || \
                                  (x->type == t_int && y->type == t_int \
                                   && x->ival % y->ival != 0) ? t_float : t_int)

#define ASSERT(x, t, m) if (x != t) error(m)
#define ASSERT_NUM(x, m) if (!(x->type == t_int || x->type == t_float || x->type == t_char)) error(m)

#define SKIP_REF_COUNT if (!refctr || n == NULL || n->rc == -1) return;

typedef enum {
   t_int,
   t_float,
   t_bool,
   t_symbol,
   t_char,
   t_pair,
   t_closure,
   t_operator,
   t_apply,
   t_lambda,
   t_cond,
   t_seq,
   t_def,
   t_cons,
   t_car,
   t_cdr
} t_type;

typedef struct pair {
   t_type type;
   struct node *car;
   struct node *cdr;
} pair;

typedef struct closure {
   struct node *args;
   struct node *body;
   struct node *where;
   struct env *env;
} closure;

typedef struct operator {
   int arity;
   struct node *arg1;
   struct node *arg2;
   union {
      struct node * (*op) (struct node *);
      struct node * (*binop) (struct node *, struct node *);
   };
} operator;

typedef struct ast {
   struct node *n1;
   struct node *n2;
   struct node *n3;
} ast;

typedef struct node {
   t_type type;
   int lineno;
   int rc;
   union {
      int ival;
      float fval;
      closure *fn;
      operator *op;
      ast *ast;
      pair *pair;
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

node *NODE_BOOL_TRUE, *NODE_BOOL_FALSE, *temp;
env *top, *tco_env;
int lineno;
symbol wildcard;

/* not a real type */
#define t_string 9999

/* memory manager statistics */
static long cnt_alloc = 0;
static long cnt_free = 0;
static long cnt_inc = 0;
static long cnt_dec = 0;

/* abstract syntax tree */
node *parse(char *filename);

/* constructors */
node *mkpair(t_type type, node *car, node* cdr);
node *mksym(char *s);
node *mkprimitive(struct node * (*primitive) (struct node *));
node *mkoperator(struct node * (*op) (struct node *), node *arg1);
node *mkbinoperator(struct node * (*binop) (struct node *, struct node *), node *arg1, node *arg2);
node *mklambda(node *params, node *body, node *where);
node *mkclosure(node *params, node *body, node *where, env *e);
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
void incref(node *n);
void decref(node *n);
node *literally(node *n);

/* evaluation */
node *eval(node *p, env *e);
node *evlis(node *list, env *env);
bool istailrecur(node *expr, symbol s);
void bind(node *args, node *params, env *fnenv, env *argenv);
binding *bindnew(symbol name, node* node);
env *envnew(env* enclosing);
env *envdel(env* e);
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

/* utils */
node *loadlib(char *name);
void error(char *msg);

#endif
