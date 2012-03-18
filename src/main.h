#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef enum { false = 0, true = 1 } bool;

#define when(pred, expr) if (pred) expr
#define MAX_SYMBOLS 1000

#define CAR(p) (p != NULL && p->pair->car != NULL ? p->pair->car : NULL)
#define CDR(p) (p != NULL && p->pair->cdr != NULL ? p->pair->cdr : NULL)
#define CADR(p) (CAR(CDR(p)))
#define EMPTY(p) (p == NULL || p->pair->car == NULL)

#define EXTRACT_NUMBER(x) (x->type == t_float ? x->fval : x->ival)
#define NUMERIC_RETURN_TYPE(x, y) (x->type == t_float || y->type == t_float ? t_float : t_int)
#define DIVIDE_RETURN_TYPE(x, y) (x->type == t_float || y->type == t_float || \
                                  (x->type == t_int && y->type == t_int \
                                   && x->ival % y->ival != 0) ? t_float : t_int)

#define ASSERT(x, t, m) if (x != t) error(m)
#define ASSERT_NUM(x, m) if (!(x->type == t_int || \
                               x->type == t_float || \
                               x->type == t_char)) error(m)
#define ASSERT_ANY(x) (x->type == t_bool && x->ival == -1)

typedef enum {
   t_int,
   t_float,
   t_bool,
   t_symbol,
   t_char,
   t_pair,
   t_closure,
   t_apply,
   t_lambda,
   t_cond,
   t_seq,
   t_def,
   t_cons,
   t_car,
   t_cdr,
   t_let,
   t_val,
   t_using,
   t_pragma,
   t_match,
   t_pattern,
   t_add,
   t_sub,
   t_mul,
   t_dvd,
   t_lt,
   t_gt,
   t_gte,
   t_lte,
   t_neq,
   t_eq,
   t_and,
   t_or,
   t_b_and,
   t_b_or,
   t_b_xor,
   t_b_lshift,
   t_b_rshift,
   t_mod,
   t_append,
   t_range,
   t_at,
   t_as,
   t_is,
   t_neg,
   t_show,
   t_reads,
   t_len,
   t_rnd,
   t_not,
   t_bnot
} t_type;

/* not a real type */
#define t_string 9999

typedef struct pair {
   t_type type;
   struct node *car;
   struct node *cdr;
} pair;

typedef struct closure {
   struct node *args;
   struct node *body;
   struct env *env;
} closure;

typedef struct ast {
   struct node *n1;
   struct node *n2;
   struct node *n3;
} ast;

typedef struct node {
   t_type type;
   struct node *hist;
   int mark;
   union {
      int ival;
      float fval;
      closure *fn;
      ast *ast;
      pair *pair;
   };
} node;

typedef int symbol;

typedef struct binding {
   symbol sym;
   struct node *node;
   struct binding *prev;
} binding;

typedef struct env {
   struct env *parent;
   binding *bind;
} env;

char *symtab[MAX_SYMBOLS];
node *NODE_BOOL_TRUE, *NODE_BOOL_FALSE, *NODE_ANY, *temp;

extern void error(char* fmt, ...);
extern void pprint(node *node);
extern void envprint(env *e, bool depth);

/* abstract syntax tree */
node *parse(char *filename);

/* constructors */
node *mkpair(t_type type, node *car, node* cdr);
node *mksym(char *s);
node *mkprimitive(struct node * (*primitive) (struct node *));
node *mkbinoperator(struct node * (*binop) (struct node *, struct node *), node *arg1, node *arg2);
node *mklambda(node *params, node *body, node *where);
node *mkint(int value);
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

/* symbol table */
symbol intern(char *string);
char *symname(symbol s);

/* memory management */
struct node *prialloc();

#endif
