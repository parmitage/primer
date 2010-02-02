#ifndef __PRIMER_H__
#define __PRIMER_H__

#define MAX_DEFS 100
#define MAX_BINDINGS_PER_FRAME 1500

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
typedef struct {
	int oper;					/* ident */
	int nops;					/* arity */
	struct nodeTag *op[1];		/* operands */
} oprNodeType;

/* represents a typed node in the AST */
typedef struct nodeTag {
	t_type type;
	int lineno;
	union {
		int ival;			/* int, bool and char */
		float fval;
		char* sval;
		oprNodeType opr;	/* composite */
	};
} node;

/*	a symbolic name bound to a value in a lexical environment */
typedef struct binding {
	char* name;
	struct nodeTag *node;
} binding;

/*	a lexical environment as described in SICP 3.2 */
typedef struct environment {
	struct environment *enclosing;
	int count;
	binding *bindings[MAX_BINDINGS_PER_FRAME];	/* TODO make dynamically resize */
} environment;

/* error message with source line number */
typedef struct error_tag {
	int line;
	char* msg;
} error;

node* stack[50000];
int stack_ptr;
node* ast;
int lineno;

/* driver for yacc */
void parse(char* filename);

/* constructors for AST nodes */
node* mkcons(int oper, int nops, ...);
node* mksym(char* s);
node* mkint(int value);
node* mkfloat(float value);
node* mkbool(int value);
node* mkchar(char value);
node* mkstr(char* value);
node* mknil();
node* mkerr(char* msg, int lineno);

/* node functions */
void nodefree(node *p);
node* node_from_string(char* value);

/* stack operations */
void push(node* node);
node* pop();

/* a Lisp like eval function */
void eval(node *p, environment* env);

/* environment related functions */
void bind(node* params, environment* env);
binding* binding_new(char* name, node* node);
environment* environment_new(environment* enclosing);
environment *environment_delete(environment* env);
void environment_extend(environment* env, binding *binding);
binding* environment_lookup(environment* env, char* ident);

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

/* utils */
void logerr(char* msg, int line);
bool file_exists(const char * filename);
node* load_std_lib();
node* create_program(node* stdlib, node* user);

#endif
