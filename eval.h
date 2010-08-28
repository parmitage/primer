#ifndef __EVAL_H__
#define __EVAL_H__

#include "main.h"

#define MAX_SYMBOLS 1000

/* supported types */ 
typedef enum {
   t_int,
   t_float,
   t_bool,
   t_symbol,
   t_char,
   t_cons,
   t_closure
} t_type;

/* represents a composite operator in the AST */
typedef struct cons {
   int oper;                  /* ident */
   int nops;                  /* arity */
   struct env *env;
   struct node *op[1];        /* operands */
} cons;

/* represents a typed node in the AST */
typedef struct node {
   t_type type;
   int lineno;
   int rc;                     /* reference count */
   union {
      int ival;                /* int, bool and char */
      float fval;
      char* sval;
      cons opr;                /* composite */
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

/* driver for yacc */
void parse(char *filename);
node *parsel(char *filename);

/* constructors for AST nodes */
node *mkcons(int oper, int nops, ...);
node *mksym(char *s);
node *mklambda(node *params, node *body, node *where, env *e);
node *mkint(int value);
node *mkfloat(float value);
node *mkbool(int value);
node *mkchar(char value);
node *mkstr(char *value);
node *node_from_string(char *value);

/* allocator */
void incref(node *n);
void decref(node *n);
void memory_alloc_error();

/* the evaluator */
node *eval(node *p, env *e);
bool function_is_tail_recursive(node *expr, symbol s);

/* environment related functions */
void bind(node *args, node *params, env *fnenv, env *argenv);
void bindp(node *args, node *list, env *fnenv);
binding* binding_new(symbol name, node* node);
env *env_new(env* enclosing);
env *env_delete(env* e);
void env_extend(env *e, binding *binding);
binding *env_lookup(env *e, symbol sym);

/* symbol table */
symbol intern(char *string);
char *symname(symbol s);

/* primitive functions */
node *car(node *node);
node *cdr(node *node);
int length(node *node);
void display(node *node);
void pprint(node *node);
bool empty(node *list);

/* operators */
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
node *append(node *list1, node *list2);
node *range(node *from, node *to);

/* library loading */
node *loadlib(char *name); 

/* error handling */
void error(char *msg);

#endif
