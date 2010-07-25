#ifndef __PRIMER_H__
#define __PRIMER_H__

#define MAX_DEFS 100
#define MAX_BINDINGS_PER_FRAME 1000

typedef enum { false = 0, true = 1 } bool;

/* supported types */ 
typedef enum {
  t_int,
  t_float,
  t_bool,
  t_symbol,
  t_char,
  t_cons,
  t_error
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

/* a symbolic name bound to a value */
typedef struct binding {
  char* name;
  struct node *node;
} binding;

/* a lexical environment as described in SICP ch.3.2 */
typedef struct environment {
  struct environment *enclosing;
  int count;
  binding *bindings[MAX_BINDINGS_PER_FRAME];
} environment;

node *NODE_EMPTY;
node *NODE_BOOL_TRUE;
node *NODE_BOOL_FALSE;
node *NODE_INT_ZERO;

node *temp, *ast;
int lineno;

/* driver for yacc */
void parse(char* filename);
node* parsel(char* filename);

/* constructors for AST nodes */
node* mkcons(int oper, int nops, ...);
node* mksym(char* s);
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

/* environment related functions */
void bind(node *args, node *params, environment *fnenv, environment *argenv);
void bindp(node *args, node *list, environment *fnenv);
binding* binding_new(char* name, node* node);
environment* environment_new(environment* enclosing);
environment *environment_delete(environment* env);
void environment_extend(environment* env, binding *binding);
binding* environment_lookup(environment* env, char* ident);
void *environment_print(environment* env);

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

/* error handling */
void error(char *msg);

/* utils */
bool file_exists(const char * filename);

#endif
