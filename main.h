#ifndef __PRIMER_MAIN_H__
#define __PRIMER_MAIN_H__

typedef enum { false = 0, true = 1 } bool;

#define when(pred, expr) if (pred) expr

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

/* command line arguments to the interpreter */
char arg_fname[1000];
bool arg_loadlib;
bool refctr;
char *arg_stdlib;
bool arg_trace;

#endif
