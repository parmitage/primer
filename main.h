#ifndef __MAIN_H__
#define __MAIN_H__

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
   t_operator,
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
   t_pattern
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
   struct node *hist;
   int mark;
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

typedef struct binding {
   symbol sym;
   struct node *node;
   struct binding *prev;
} binding;

typedef struct env {
   struct env *parent;
   binding *bind;
} env;

symbol GC_DISABLE;

extern void error(char* fmt, ...);
extern void pprint(node *node);
extern void envprint(env *e, bool depth);

#endif
