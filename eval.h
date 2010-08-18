#ifndef __EVAL_H__
#define __EVAL_H__

#define MAX_DEFS 100
#define MAX_SYMBOLS 1000

typedef enum { false = 0, true = 1 } bool;

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
  struct environment *env;
  struct node *op[1];        /* operands */
} cons;

/* represents a typed node in the AST */
typedef struct node {
  t_type type;
  int lineno;
  union {
    int ival;                /* int, bool and char */
    float fval;
    char* sval;
    cons opr;                /* composite */
  };
} node;

typedef int symbol;
char *symbol_table[MAX_SYMBOLS];

typedef struct binding {
   symbol sym;
   struct node *node;
   struct binding *prev;
} binding;

typedef struct environment {
   struct environment *enclosing;
   binding *bind;
} environment;

node *NODE_EMPTY;
node *NODE_BOOL_TRUE;
node *NODE_BOOL_FALSE;
node *NODE_INT_ZERO;

environment *global;
node *temp, *ast;
int lineno;
int wildcard;

/* driver for yacc */
void parse(char* filename);
node* parsel(char* filename);

/* constructors for AST nodes */
node* mkcons(int oper, int nops, ...);
node* mksym(char* s);
node *mklambda(node *params, node *body, node *where, environment *e);
node* mkint(int value);
node* mkfloat(float value);
node* mkbool(int value);
node* mkchar(char value);
node* mkstr(char* value);
node* node_from_string(char* value);

/* allocator */
void memory_alloc_error();
void nodefree(node *p);

/* the evaluator */
node *eval(node *p, environment* env);
bool function_is_tail_recursive(node *expr, symbol s);

/* environment related functions */
void bind(node *args, node *params, environment *fnenv, environment *argenv);
void bindp(node *args, node *list, environment *fnenv);
binding* binding_new(symbol name, node* node);
environment* environment_new(environment* enclosing);
environment *environment_delete(environment* env);
void environment_extend(environment* env, binding *binding);
binding* environment_lookup(environment* env, symbol sym);

/* symbol table */
symbol intern(char *string);
char *symbol_name(symbol s);

/* primitives */
node* car(node* node);
node* cdr(node* node);
int length(node* node);
int node_type(node* node);
void display(node* node);
void display_primitive(node* node);
bool empty(node *list);

/* operators */
node* add(node* x, node* y);
node* sub(node* x, node* y);
node* mul(node* x, node* y);
node* dvd(node* x, node* y);
node* lt(node* x, node* y);
node* gt(node* x, node* y);
node* lte(node* x, node* y);
node* gte(node* x, node* y);
node* eq(node* x, node* y);
node* neq(node* x, node* y);
node* list_eq(node* l1, node* l2);
node* and(node* x, node* y);
node* or(node* x, node* y);
node* not(node* node);
node* mod(node* x, node* y);
node* append(node* list1, node* list2);
node *range(node *from, node *to);

/* library loading */
node* library_load(char* name);
bool file_exists(const char * filename);

/* error handling */
void error(char *msg);

#endif
