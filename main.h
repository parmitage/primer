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
	t_char,
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
    int ival;				/* int, bool and char */
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

/* constructors for AST nodes */
node *opr(int oper, int nops, ...);
node *sym(char* s);
node *con(int value);
node *fpval(float value);
node *boolval(int value);
node* node_make_string(char* value);
node* mkchar(char c);
node* nil();

/* node functions */
void nodefree(node *p);
node* node_from_string(char* value);

/* print an error with its line number */
void logerr(char* msg, int line);

bool file_exists(const char * filename);

#endif
