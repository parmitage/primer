#ifndef __QTL_INTERPRETER_H__
#define __QTL_INTERPRETER_H__

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

/* loads the standard library off disk */
node* load_std_lib();

/* merges the standard library and the users code into a program */
node* create_program(node* stdlib, node* user);

/* a Lisp like eval function */
void eval(node *p, environment* env);

/* bind a parameter to a value in an environment */
void bind(node* params, environment* env);

/* constructor for binding object */
binding* binding_new(char* name, node* node);

/* creates a new lexical environment */
environment* environment_new(environment* enclosing);

/* destroyes the supplied lexical environment */
environment *environment_delete(environment* env);

/* extends an environment with a binding */
void environment_extend(environment* env, binding *binding);

/* looks up a binding in an environment or it's chain of parents */
binding* environment_lookup(environment* env, char* ident);

#endif
