#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "interpreter.h"
#include "y.tab.h"

int main(int argc, char** argv)
{
  if (argc != 2)
    {
      printf("usage: primer <filename>\n");
      return -1;
    }

  NODE_NIL = mknil();
  NODE_BOOL_TRUE = mkbool(true);
  NODE_BOOL_FALSE = mkbool(false);
  NODE_INT_ZERO = mkint(0);
	
  node* stdlib = load_std_lib();
  lineno = 1;
  parse(argv[1]);
  node* program = create_program(stdlib, ast);
  eval(program, NULL);
	
  return 0;
}

void eval(node *p, environment* env)
{
  if (!p)
    return;

 eval_start:

  switch(p->type)
    {
    case t_int:
    case t_float:
    case t_bool:
    case t_char:
    case t_nil:
      push(p);
      break;

    case t_error:
      {
        logerr(p->sval, p->lineno);
        break;
      }

    case t_symbol:
      {
        binding *b = environment_lookup(env, p->sval);
			
        if (b != NULL)
          eval(b->node, env);
        else
          logerr("unbound symbol", p->lineno);
      }

    case t_cons:
      {
        switch(p->opr.oper)
          {
          case PROG:
            {
              /* create the global lexical environment */
              env = environment_new(NULL);
              eval(p->opr.op[0], env);
              env = environment_delete(env);
              break;
            }
				
          case DEF:
            {
              break;
            }
				
          case ASSIGN:
            {
              char* name = p->opr.op[0]->sval;
              eval(p->opr.op[1], env);
              binding* binding = binding_new(name, pop());
              environment_extend(env, binding);
              break;
            }
				
          case LAMBDA:
            {
              /* Direct evaluation of lambda, presumably when passed as
                 an anonymous function to another function rather than
                 via a funcall, means pushing the lambda back onto the
                 stack ready to be evaluated. */
              push(p);
              break;
            }
				
          case FUNCALL:
            {
              eval(p->opr.op[0], env);
              node* fn = pop();
              node *params = p->opr.op[1];

              while (params != NULL)
                {
                  if (params->opr.nops > 0)
                    {
                      eval(params->opr.op[0], env);
                      params = params->opr.op[1];
                    }
                  else
                    params = NULL;
                }
	      
              environment *ext = environment_new(env);
              bind(fn->opr.op[0], ext);
              p = fn->opr.op[1];
              env = ext;
              goto eval_start;

              break;
            }
				
          case LIST:
            {
              /* Because we can store symbols in lists we evaluate the
                 list contents. Probably could make this significantly
                 more efficient! */
              switch (p->opr.nops)
                {
                case 0:
                  push(mkcons(LIST, 0));
                  break;
                case 1:
                  eval(p->opr.op[0], env);
                  push(mkcons(LIST, 1, pop()));
                  break;
                case 2:
                  eval(p->opr.op[0], env);
                  eval(p->opr.op[1], env);
                  push(mkcons(LIST, 2, pop(), pop()));
                  break;
                }
              break;
            }
				
          case STRING:
            {
              push(p);
              break;
            }
				
          case SHOW:
            {
              eval(p->opr.op[0], env);
              node* val = pop();
              display(val);
              break;
            }
				
          case HEAD:
            {
              eval(p->opr.op[0], env);
              node* list = pop();
              push(car(list));
              break;	
            }
				
          case TAIL:
            {
              eval(p->opr.op[0], env);
              node* list = pop();
              push(cdr(list));
              break;
            }
				
          case CONS:
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(cons(pop(), pop()));
              break;
            }
				
          case ';':
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              break;
            }
				

          case IF:
            {
              eval(p->opr.op[0], env);					
              node *pred = pop();
					
              if (pred->ival > 0)
                {
                  environment *ext = environment_new(env);
                  p = p->opr.op[1];
                  env = ext;
                  goto eval_start;
                }
              else if (p->opr.nops > 3)
                {
                  environment *ext = environment_new(env);
                  p = p->opr.op[2];
                  env = ext;
                  goto eval_start;
                }
					
              break;
            }
				
          case '+':
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(add(pop(), pop()));
              break;
            }
				
          case '-':
            {
              if (p->opr.nops == 1)
                {
                  eval(p->opr.op[0], env);
                  push(sub(mkint(0), pop()));
                }
              else if (p->opr.nops == 2)
                {
                  eval(p->opr.op[0], env);
                  eval(p->opr.op[1], env);
                  push(sub(pop(), pop()));
                }

              break;
            }
				
          case '*':
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(mul(pop(), pop()));
              break;
            }
				
          case '/':
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(dvd(pop(), pop()));
              break;
            }
				
          case '<':
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(lt(pop(), pop()));
              break;
            }
				
          case '>':
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(gt(pop(), pop()));
              break;
            }
				
          case GE:
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(gte(pop(), pop()));
              break;
            }
				
          case LE:
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(lte(pop(), pop()));
              break;
            }
				
          case NE:
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(neq(pop(), pop()));
              break;	
            }
				
          case EQ:
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(eq(pop(), pop()));
              break;
            }
				
          case AND:
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(and(pop(), pop()));
              break;
            }
				
          case OR:
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(or(pop(), pop()));
              break;
            }
				
          case MOD:
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(mod(pop(), pop()));
              break;
            }
				
          case APPEND:
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(append(pop(), pop()));
              break;
            }
				
          case RANGE:
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              push(range(pop(), pop()));
              break;
            }
				
          case NOT:
            {
              eval(p->opr.op[0], env);
              push(not(pop()));
              break;
            }
				
          case TYPE:
            {
              int t = node_type(p->opr.op[0]);
					
              if (t == t_symbol)
                {
                  binding *b = environment_lookup(env, p->opr.op[0]->sval);
						
                  if (b != NULL)
                    t = b->node->type;
                  else
                    t = -1;
                }

              push(mkint(t));
              break;
            }
          }
      }
    }
}

void bind(node* params, environment* env)
{
  if (params != NULL)
    {		
      if (params->opr.nops > 1)
        {
          bind(params->opr.op[1], env);
        }
		
      if (params->opr.nops > 0)
        {
          binding* binding = binding_new(params->opr.op[0]->sval, pop());
          environment_extend(env, binding);
        }
    }
}

binding* binding_new(char* name, node* node)
{
  size_t size = sizeof(binding) + (strlen(name) * sizeof(char)) + sizeof(node);
  binding* bind = (binding*)malloc(size);
	
  bind->name = strdup(name);
  bind->node = node;
  return bind;
}

environment* environment_new(environment* enclosing)
{
  size_t size = sizeof(environment) + MAX_BINDINGS_PER_FRAME * sizeof(binding*);
  environment* env = (environment*)malloc(size);
	
  /* enclosing environment will be NULL for global environment */
  env->enclosing = enclosing;
	
  return env;
}

environment *environment_delete(environment* env)
{
  environment *enclosing = env->enclosing;
	
  /* note that we free the string allocated to hold the binding name and
     the pointer itself but we DO NOT free the node that the binding
     points to as this is a reference into the AST */
	
  free(env);
	
  return enclosing;
}

void environment_extend(environment* env, binding *binding)
{
  hash_insert(env->bindings, binding->name, binding);
}

binding* environment_lookup(environment* env, char* name)
{
  environment* top = env;

  while (env != NULL)
    {
      binding* b = hash_retreive(env->bindings, name);

      if (b != NULL)
        {
          environment_extend(top, b);
          return b;
        }

      env = env->enclosing;
    }
  
  return NULL;
}

node *mkint(int value)
{
  node *p;
  size_t nodeSize = sizeof(node) + sizeof(int);
	
  if ((p = malloc(nodeSize)) == NULL)
    abort();
	
  p->type = t_int;
  p->ival = value;
  p->lineno = lineno;
	
  return p;
}

node *mkfloat(float value)
{
  node *p;
  size_t nodeSize = sizeof(node) + sizeof(float);
	
  if ((p = malloc(nodeSize)) == NULL)
    abort();
	
  p->type = t_float;
  p->fval = value;
  p->lineno = lineno;
	
  return p;
}

node *mkbool(int value)
{
  node *p;
  size_t nodeSize = sizeof(node) + sizeof(int);
	
  if ((p = malloc(nodeSize)) == NULL)
    abort();
	
  p->type = t_bool;
  p->ival = value;
  p->lineno = lineno;
	
  return p;
}

node* mkchar(char c)
{
  node* p;
		
  if ((p = malloc(sizeof(node) + sizeof(char))) == NULL)
    abort();

  p->type = t_char;
  p->lineno = lineno;
  p->ival = c;
	
  return p;
}

node* mkstr(char* value)
{
  int srclen = strlen(value);
  int destlen = srclen - 1;
  int copylen = srclen - 2;
  char* temp = (char*)malloc(destlen * sizeof(char) + 1);
  strncpy(temp, value + 1, copylen);
  temp[copylen] = '\0';
  return node_from_string(temp);
}

node* node_from_string(char* value)
{
  int len = strlen(value);

  if (len > 1)
    return mkcons(STRING, 2, mkchar(value[0]), node_from_string(value + 1));
  else
    return mkcons(STRING, 1, mkchar(value[0]));
}

node* mknil()
{
  node *p;
  size_t nodeSize = sizeof(node) + sizeof(int);
	
  if ((p = malloc(nodeSize)) == NULL)
    abort();
	
  p->type = t_nil;
  p->lineno = lineno;
	
  return p;
}

node *mksym(char* s)
{
  node *p;
  size_t nodeSize = sizeof(node) + (strlen(s) * sizeof(char*));
	
  if ((p = malloc(nodeSize)) == NULL)
    abort();
	
  p->type = t_symbol;
  p->sval = strdup(s);
  p->lineno = lineno;
	
  return p;
}

node *mkcons(int oper, int nops, ...)
{
  if (nops == 0)
    return mknil();

  va_list ap;
  node *p;
  int i;
  size_t nodeSize = sizeof(node)
    + sizeof(oprNodeType)
    + nops * sizeof(node*)
    + sizeof(environment*);
	
  if ((p = malloc(nodeSize)) == NULL)
    abort();
	
  p->type = t_cons;
  p->lineno = lineno;
  p->opr.oper = oper;
  p->opr.nops = nops;
	
  va_start(ap, nops);
	
  for (i = 0; i < nops; i++)
    p->opr.op[i] = va_arg(ap, node*);
	
  va_end(ap);
	
  return p;
}

node* mkerr(char* msg, int lineno)
{
  node *p;
  size_t nodeSize = sizeof(node) + (strlen(msg) * sizeof(char*));
	
  if ((p = malloc(nodeSize)) == NULL)
    abort();
	
  p->type = t_error;
  p->sval = strdup(msg);
  p->lineno = lineno;
	
  return p;
}

void nodefree(node *p)
{
  if (!p)
    return;

  int i;
	
  if (p->type == t_cons)
    {
      for (i = 0; i < p->opr.nops; i++)
        nodefree(p->opr.op[i]);
    }
	
  free (p);
}

void push(node* node)
{
  stack[stack_ptr++] = node;
}

node* pop()
{
  if (stack_ptr > 0)
    return stack[--stack_ptr];
  else
    return mknil();
}

node* car(node* node)
{
  if (node == NULL)
    return mknil();
  else if (node->type == t_nil)
    return node;
  else
    return node->opr.op[0];
}

node* cdr(node* node)
{
  if (node == NULL)
    return mknil();
  else if (node->opr.nops > 0 && node->opr.op[1] != NULL)
    {
      return node->opr.op[1];
    }
  else
    return mknil();
}

node* cons(node* list, node* n)
{
  if (list->type == t_nil)
    return mkcons(LIST, 1, n, NULL);
  else if (n->type == t_nil)
    return mkcons(LIST, 1, list, NULL);
  else
    return mkcons(LIST, 2, n, list);
}

node* append(node* list1, node* list2)
{
  if (list2 == NULL || list2->type == t_nil)
    return list1;

  if (list1 == NULL || list1->type == t_nil)
    return list2;

  node* r = list1;
  node* n = list1;
	
  while (n != NULL && n->type != t_nil && n->opr.op[1] != NULL)
    {
      n = n->opr.op[1];
    }
	
  n->opr.nops = 2;
  n->opr.op[1] = list2;
	
  return r;
}

int node_type(node* node)
{
  return node->type;
}

void display_primitive(node* node, int depth)
{
  switch (node->type)
    {
    case t_int:
      printf("%i", node->ival);
      break;
    case t_float:
      printf("%g", node->fval);
      break;
    case t_nil:
      printf("nil");
      break;
    case t_bool:
      printf("%s", node->ival > 0 ? "true" : "false");
      break;
    case t_char:
      printf("%c", node->ival);
      break;
		
    case t_cons:
      {
        switch (node->opr.oper)
          {
          case STRING:
            {
              while (node != NULL)
                {
                  if (node->opr.nops > 0)
                    {
                      printf("%c", node->opr.op[0]->ival);
                      node = node->opr.op[1];
                    }
                  else
                    node = NULL;
                }
					
              break;
            }
				
          case LIST:
            {
              if (depth == 0)
                printf("[");
						
              display_primitive(node->opr.op[0], depth++);
					
              if (node->opr.nops > 1 && node->opr.op[1] != NULL)
                {
                  printf(",");
                  if (node->opr.op[1]->opr.op[0] != NULL &&
                      node->opr.op[1]->opr.op[0]->type == t_cons)
                    printf("[");
							
                  display_primitive(node->opr.op[1], depth);
                }
					
              if (node->opr.op[1] == NULL)
                printf("]");
							
              --depth;
					
              break;
            }
				
          case LAMBDA:
            {
              printf("LAMBDA");
              break;
            }
          }
			
        break;
      }
		
    case t_symbol:
      printf("%s", node->sval);
      break;
			
    case t_error:
      logerr(node->sval, node	->lineno);
      break;
    }		
}

void display(node* node)
{
  if (node == NULL)
    return;
		
  display_primitive(node, 0);
		
  printf("\n");
}

node* add(node* x, node* y)
{
  if (x->type == t_float)
    {
      if (y->type == t_float)
        return mkfloat(x->fval + y->fval);
      else
        return mkfloat(x->fval + y->ival);
    }
  else
    {
      if (y->type == t_float)
        return mkfloat(x->ival + y->fval);
      else
        return mkint(x->ival + y->ival);
    }		
}

node* sub(node* x, node* y)
{
  if (x->type == t_float)
    {
      if (y->type == t_float)
        return mkfloat(x->fval - y->fval);
      else
        return mkfloat(x->fval - y->ival);
    }
  else
    {
      if (y->type == t_float)
        return mkfloat(x->ival - y->fval);
      else
        return mkint(x->ival - y->ival);
    }
}

node* mul(node* x, node* y)
{
  if (x->type == t_float)
    {
      if (y->type == t_float)
        return mkfloat(x->fval * y->fval);
      else
        return mkfloat(x->fval * y->ival);
    }
  else
    {
      if (y->type == t_float)
        return mkfloat(x->ival * y->fval);
      else
        return mkint(x->ival * y->ival);
    }		
}

node* dvd(node* x, node* y)
{
  if (x->type == t_float)
    {
      if (y->type == t_float)
        return mkfloat(x->fval / y->fval);
      else
        return mkfloat(x->fval / y->ival);
    }
  else
    {
      if (y->type == t_float)
        return mkfloat(x->ival / y->fval);
      else
        return mkint(x->ival / y->ival);
    }		
}

node* lt(node* x, node* y)
{
  if (x->type == t_float)
    {
      if (y->type == t_float)
        return x->fval < y->fval ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
      else
        return x->fval < y->ival ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
    }
  else
    {
      if (y->type == t_float)
        return x->ival < y->fval ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
      else
        return x->ival < y->ival ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
    }		
}

node* gt(node* x, node* y)
{
  return not(lte(x, y));
}

node* lte(node* x, node* y)
{
  if (x->type == t_float)
    {
      if (y->type == t_float)
        return x->fval <= y->fval ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
      else
        return x->fval <= y->ival ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
    }
  else
    {
      if (y->type == t_float)
        return x->ival <= y->fval ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
      else
        return x->ival <= y->ival ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
    }		
}

node* gte(node* x, node* y)
{
  return not(lt(x, y));
}

node* list_eq(node* l1, node* l2)
{
  if (l1 == NULL && l2 == NULL)
    return NODE_BOOL_TRUE;
	
  if (l1->type == t_nil && l2->type == t_nil)
    return NODE_BOOL_TRUE;

  if (l1->opr.nops != l2->opr.nops)
    return NODE_BOOL_FALSE;
	
  if (eq(l1->opr.op[0], l2->opr.op[0])->ival == true)
    {
      if (l1->opr.nops == 2 && l2->opr.nops == 2)
        return list_eq(l1->opr.op[1], l2->opr.op[1]);
      else
        return NODE_BOOL_TRUE;
    }
  else
    return NODE_BOOL_FALSE;
}

node* eq(node* x, node* y)
{
  if (x->type != y->type)
    return NODE_BOOL_FALSE;
		
  if (x->type == t_int || x->type == t_bool)
    return x->ival == y->ival ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
		
  if (x->type == t_float)
    return x->fval == y->fval ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;

  if (x->type == t_char)
    return x->ival == y->ival ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
	
  if (x->type == t_nil)
    return NODE_BOOL_TRUE;
	
  if (x->type == t_cons)
    return list_eq(x, y);
		
  return NODE_BOOL_FALSE;
}

node* neq(node* x, node* y)
{
  return(not(eq(x, y)));
}

node* and(node* x, node* y)
{
  if (x->type == t_bool && y->type == t_bool)
    {
      return mkbool(x->ival && y->ival);
    }
  else
    {
      // TODO probably should throw error
      return NODE_BOOL_FALSE;
    }
}

node* or(node* x, node* y)
{
  if (x->type == t_bool && y->type == t_bool)
    {
      return mkbool(x->ival || y->ival);
    }
  else
    {
      // TODO probably should throw error
      return NODE_BOOL_FALSE;
    }
}

node* not(node* node)
{
  if (node->ival == true)
    return NODE_BOOL_FALSE;
  else
    return NODE_BOOL_TRUE;
}

node* mod(node* x, node* y)
{
  if (x->type == t_int && y->type == t_int)
    return mkint(x->ival % y->ival);
  else
    return mkerr("% operator only supports integers", x->lineno);
}

node* range(node* s, node* e)
{
  node* l;

  if (s->type == t_int || e->type == t_int)
    {
      l = mknil();
			
      for (int i = e->ival; i >= s->ival; --i)
        l = cons(l, mkint(i));
    }
  else
    {
      l = mkerr(".. operator only supports integer ranges", s->lineno);
    }
	
  return l;
}

node* load_std_lib()
{
  char* libpath;
  libpath = getenv("PRIMER_LIBRARY_PATH");
  	
  if (libpath == NULL)
    {
      printf("The environment variable PRIMER_LIBRARY_PATH has not been set\n");
      exit(-1);
    }
    
  strcat(libpath, "stdlib.pri");
    
  if (!file_exists(libpath))
    {
      printf("Unable to find standard library %s\n", libpath);
      exit(-1);
    }
	
  parse(libpath);
  return ast->opr.op[0];
}

node* create_program(node* stdlib, node* user)
{
  node* level1 = mkcons(';', 2, stdlib, ast->opr.op[0]);
  return mkcons(PROG, 1, level1);
}

void logerr(char* msg, int line)
{
  printf("error at line %i: %s\n", line, msg);
  exit(1);
}

void dbg(char* msg)
{
  printf("%s\n", msg);
}

bool file_exists(const char * path)
{
  FILE *istream;
	
  if((istream = fopen(path, "r")) == NULL)
    {
      return false;
    }
  else
    {
      fclose(istream);
      return true;
    }
}

/* hashtable stuff */

unsigned int hash(char* str)
{
  unsigned int hash = 0;
  int c;

  while (c = *str++)
    hash = c + (hash << 6) + (hash << 16) - hash;

  return hash % MAX_BINDINGS_PER_FRAME;
}

void hash_insert(binding* hashtable[], char* key, binding* value)
{
  hashtable[hash(key)] = value;
}

binding* hash_retreive(binding* hashtable[], char* key)
{
  return hashtable[hash(key)];
}

