#ifndef __PRIMER_H__
#define __PRIMER_H__

#define MAX_DEFS 100
#define MAX_BINDINGS_PER_FRAME 1000

typedef enum { false = 0, true = 1 } bool;

/* supported types */ 
typedef enum {
  t_nil,
  t_int,
  t_float,
  t_bool,
  t_symbol,
  t_char,
  t_cons,
  t_error
} t_type;

/* represents a composite operator in the AST */
typedef struct oprNodeType {
  int oper;					/* ident */
  int nops;					/* arity */
  struct nodeTag *op[1];		/* operands */
} oprNodeType;

/* represents a typed node in the AST */
typedef struct nodeTag {
  t_type type;
  int lineno;
  struct environment *env;
  union {
    int ival;			/* int, bool and char */
    float fval;
    char* sval;
    oprNodeType opr;	/* composite */
  };
} node;

/* a symbolic name bound to a value in a lexical environment */
typedef struct binding {
  char* name;
  struct nodeTag *node;
} binding;

/* a lexical environment as described in SICP ch.3.2 */
typedef struct environment {
  struct environment *enclosing;
  int count;
  binding *bindings[MAX_BINDINGS_PER_FRAME];
} environment;

/* error message with source line number */
typedef struct error_tag {
  int line;
  char* msg;
} error;

node* NODE_NIL;
node* NODE_BOOL_TRUE;
node* NODE_BOOL_FALSE;
node* NODE_INT_ZERO;

node* stack[50000];
int stack_ptr;
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
node* mknil();
node* node_from_string(char* value);

/* allocator */
void memory_alloc_error();
void nodefree(node *p);

/* stack operations */
void push(node* node);
node* pop();

/* the evaluator */
void eval(node *p, environment* env);
bool cond(node *p, environment *env, bool match);
int length(node* node);

/* environment related functions */
node* bind(node *args, node *params, environment *fnenv, environment *argenv);
binding* binding_new(char* name, node* node);
environment* environment_new(environment* enclosing);
environment *environment_delete(environment* env);
void environment_extend(environment* env, binding *binding);
binding* environment_lookup(environment* env, char* ident);
void *environment_print(environment* env);

/* special forms */
node* car(node* node);
node* cdr(node* node);
node* cons(node* list, node* n);
int node_type(node* node);
void display(node* node);
void display_primitive(node* node, int depth);

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
node* range(node* s, node* e);

/* library loading */
node* library_load(char* name);

/* error handling */
void error_log(char *msg, node *node);

/* utils */
bool file_exists(const char * filename);

#endif
