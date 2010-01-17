#ifndef __QTL_QTL_H__
#define __QTL_QTL_H__

#define MAX_DEFS 100
#define MAX_BINDINGS_PER_FRAME 1500

typedef enum { false = 0, true = 1 } bool;

/* the supported types of XYZ */ 
typedef enum {
	t_nil,
	t_int,
	t_float,
	t_bool,
	t_symbol,
	t_cons
} t_type;

/* represents a composite operator in the AST */
typedef struct {
  int oper;					/* ident */
  int nops;					/* arity */
  struct nodeTag *op[1];	/* operands */
} oprNodeType;

/* represents a typed node in the AST */
typedef struct nodeTag {
  t_type type;
  int lineno;
  union {
    int ival;
    float fval;
    char* sval;
    oprNodeType opr;        /* composite */
  };
} node;

/* error message with source line number */
typedef struct error_tag {
  int line;
  char* msg;
} error;

node* ast;
int lineno;

/* driver for yacc */
void parse(char* filename);

/* constructor for non-primitive AST nodes (LAMBDA, FUNCALL, IF, ...) */
node *opr(int oper, int nops, ...);

/* constructor for literal symbolic AST nodes */
node *sym(char* s);

/* constructor for literal integer AST nodes */
node *con(int value);

/* constructor for literal floating point AST nodes */
node *fpval(float value);

/* constructor for literal boolean AST nodes */
node *boolval(int value);

/* constructor for NIL valued AST nodes */
node* nil();

/* recursively frees a node and its children */
void nodefree(node *p);

/* print an error with its line number */
void logerr(char* msg, int line);

#endif
