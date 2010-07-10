#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "eval.h"
#include "y.tab.h"

int main(int argc, char** argv)
{
  if (argc != 2)
    {
      printf("usage: primer <filename>\n");
      return -1;
    }

  NODE_EMPTY = mkcons(LIST, 0);
  NODE_BOOL_TRUE = mkbool(true);
  NODE_BOOL_FALSE = mkbool(false);
  NODE_INT_ZERO = mkint(0);
	
  lineno = 1;
  parse(argv[1]);
  eval(ast, NULL);
  return 0;
}

node *eval(node *p, environment* env)
{
  if (!p)
    return NODE_BOOL_FALSE;

 eval_start:

  switch(p->type)
    {
    case t_int:
    case t_float:
    case t_bool:
    case t_char:
      return p;

    case t_symbol:
      {
        binding *b = environment_lookup(env, p->sval);
			
        if (b != NULL)
          return eval(b->node, env);
        else
          error("unbound symbol");
      }

    case t_cons:
      {
        switch(p->opr.oper)
          {
          case PROG:
            {
              env = environment_new(NULL);
              eval(library_load("Library"), env);
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              env = environment_delete(env);
              break;
            }

          case PAREN:
            return eval(p->opr.op[0], env);

          case DEF:
            {
              char* name = p->opr.op[0]->sval;
              binding* binding;

              /* evalaute the RHS unless it's a closure */
              if (p->opr.op[1]->type == t_cons &&
                  p->opr.op[1]->opr.oper == APPLY)
                {
                  char *symbol = p->opr.op[1]->opr.op[0]->sval;
                  struct binding *b = environment_lookup(env, symbol);
                  
                  if (b != NULL)
                    {
                      node *n = b->node;
                    
                      if (n->type == t_cons && n->opr.oper == LAMBDA
                          && n->opr.op[1]->type == t_cons
                          && n->opr.op[1]->opr.oper == LAMBDA)
                        binding = binding_new(name, p->opr.op[1]);
                      else
                        binding = binding_new(name, eval(p->opr.op[1], env));
                    }
                }
              else
                binding = binding_new(name, eval(p->opr.op[1], env));

              environment_extend(env, binding);
              break;
            }
				
          case LAMBDA:
            {
	      /* clone environment so that f(f(x)) works */
	      node *clone;

              if (p->opr.nops == 2)
                clone = mkcons(LAMBDA, 2, p->opr.op[0], p->opr.op[1]);
              else
                clone = mkcons(LAMBDA, 3, p->opr.op[0], p->opr.op[1], p->opr.op[2]);

	      clone->env = environment_new(env);
	      return clone;
            }

          case APPLY:
            {
              node* fn = eval(p->opr.op[0], env);

              /* bind parameters */
              node *params = p->opr.op[1];
              bind(fn->opr.op[0], params, fn->env, env);

               /* evaluate where clause */
              if (fn->opr.nops == 3)
                eval(fn->opr.op[2], fn->env);

              /* evaluate function body */
              p = fn->opr.op[1];
              env = fn->env;
              goto eval_start;
            }

          case LIST:
            {
              /* because we can store symbols in lists we evaluate the contents */
              switch (p->opr.nops)
                {
                case 0:
                  return mkcons(LIST, 0);
                case 1:
                  return mkcons(LIST, 1, eval(p->opr.op[0], env));
                case 2:
                  return mkcons(LIST, 2, eval(p->opr.op[0], env), eval(p->opr.op[1], env));
                }
            }
            
          case STRING:
            {
              switch (p->opr.nops)
                {
                case 0:
                  return mkcons(STRING, 0);
                case 1:
                  return mkcons(STRING, 1, eval(p->opr.op[0], env));
                case 2:
                  return mkcons(STRING, 2, eval(p->opr.op[0], env), eval(p->opr.op[1], env));
                }
            }
				
          case SHOW:
            {
              node* val = eval(p->opr.op[0], env);
              display(val);
              return val;
            }
				
          case ';':
            {
              eval(p->opr.op[0], env);
              eval(p->opr.op[1], env);
              break;
            }

            case IF:
            {              			
              node *pred = eval(p->opr.op[0], env);

              if (pred->type != t_bool)
                error("type of predicate is not boolean");
					
              if (pred->ival > 0)
                p = p->opr.op[1];
              else
                p = p->opr.op[2];

              goto eval_start;
            }

          case LENGTH:
            {
              node* val = eval(p->opr.op[0], env);
              return mkint(length(val));
            }

          case NTH:
            {
              node *list = eval(p->opr.op[0], env);
              int index = eval(p->opr.op[1], env)->ival, n = 0;
              bool found = false;

              while (!found)
                {
                  if (list == NULL || index < 0)
                    {
                      return NODE_EMPTY;
                      found = true;
                    }
                  else if (index == n)
                    {
                      return car(list);
                      found = true;
                    }
                  else
                    {
                      list = cdr(list);
                      ++n;
                    }
                }
              
              break;
            }
				
          case '+':
            return add(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case '-':
            {
              if (p->opr.nops == 1)
                return sub(mkint(0), eval(p->opr.op[0], env));
              else if (p->opr.nops == 2)
                return sub(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
            }
				
          case '*':
            return mul(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case '/':
            return dvd(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case '<':
            return lt(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case '>':
            return gt(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case GE:
            return gte(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case LE:
            return lte(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case NE:
            return neq(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case EQ:
            return eq(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case AND:
            return and(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case OR:
            return or(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case MOD:
            return mod(eval(p->opr.op[0], env), eval(p->opr.op[1], env));

          case APPEND:
            return append(eval(p->opr.op[0], env), eval(p->opr.op[1], env));

          case RANGE:
            return range(eval(p->opr.op[0], env), eval(p->opr.op[1], env));
				
          case NOT:
            return not(eval(p->opr.op[0], env));
				
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

              return mkint(t);
            }
          }
      }
    }
}

int length(node* node)
{
  if (node->type == t_cons)
    {
      if (node->opr.nops > 1 && node->opr.op[1] != NULL)
        return 1 + length(node->opr.op[1]);
      else
        return node->opr.nops;
    }
  else
    return 1;
}

void bind(node *args, node *params, environment *fnenv, environment *argenv)
{
  if (params != NULL)
    {		
      if (params->opr.nops > 1)
        {
          bind(args->opr.op[1], params->opr.op[1], fnenv, argenv);
        }
		
      if (params->opr.nops > 0)
        {
          node *n = eval(params->opr.op[0], argenv);
          
          if (args->opr.op[0]->type == t_symbol)
            {
              binding* binding = binding_new(args->opr.op[0]->sval, n);
              environment_extend(fnenv, binding);
            }
          else if (args->opr.op[0]->type == t_cons && args->opr.op[0]->opr.oper == CONS)
            bindp(args->opr.op[0], n, fnenv);
        }
    }
}

void bindp(node *args, node *list, environment *fnenv)
{
  node *head = car(list);
  node *rest = cdr(list);

  if (strcmp(args->opr.op[0]->sval, "_") != 0)
    {
      binding *headb = binding_new(args->opr.op[0]->sval, head);
      environment_extend(fnenv, headb);
    }

  if (args->opr.op[1]->type == t_symbol)
    {
      if (strcmp(args->opr.op[1]->sval, "_") != 0)
        {
          binding *restb = binding_new(args->opr.op[1]->sval, rest);
          environment_extend(fnenv, restb);
        }
    }
  else
    {
      bindp(args->opr.op[1], rest, fnenv);
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
  env->count = 0;
	
  return env;
}

environment *environment_delete(environment* env)
{
  environment *enclosing = env->enclosing;
	
  /* note that we free the string allocated to hold the binding name and
     the pointer itself but we DO NOT free the node that the binding
     points to as this is a reference into the AST */
	
  //free(env);
	
  return enclosing;
}

void environment_extend(environment* env, binding *binding)
{
  char *sym = binding->name;

  for (int i = 0; i < env->count; ++i)
    {
      if (strcmp(sym, env->bindings[i]->name) == 0)
        {
          /* binding exists in this environment so replace it */
          env->bindings[i] = binding;
          return;
        }
    }

  /* binding wasn't found so create a new one */
  env->bindings[env->count++] = binding;
}

binding* environment_lookup(environment* env, char* name)
{
  environment *top = env;
  bool depth = false;

  while (env != NULL)
    {
      for (int i = 0; i < env->count; ++i)
	{
	  if (strcmp(env->bindings[i]->name, name) == 0)
	    {
              /* lift a binding into this environment */
              if (depth)
                environment_extend(top, env->bindings[i]);

	      return env->bindings[i];
	    }
	}

      env = env->enclosing;
      depth = true;
    }
  
  return NULL;
}

void *environment_print(environment* env)
{
  while (env != NULL)
    {
      for (int i = 0; i < env->count; ++i)
	{
          printf("%s\n", env->bindings[i]->name);
	}
      printf("^^^^\n");
      env = env->enclosing;
    }
}

node *mkint(int value)
{
  node *p;
  int size = sizeof(struct nodeTag) + sizeof(int);

  if ((p = (struct nodeTag *)malloc(size)) == NULL)
    memory_alloc_error();
	
  p->type = t_int;
  p->ival = value;
  p->lineno = lineno;
	
  return p;
}

node *mkfloat(float value)
{
  node *p;
  size_t size = sizeof(struct nodeTag) + sizeof(float);
	
  if ((p = (struct nodeTag *)malloc(size)) == NULL)
    memory_alloc_error();
  
  p->type = t_float;
  p->fval = value;
  p->lineno = lineno;
	
  return p;
}

node *mkbool(int value)
{
  node *p;
  size_t size = sizeof(struct nodeTag) + sizeof(int);
	
  if ((p = (struct nodeTag *)malloc(size)) == NULL)
    memory_alloc_error();
	
  p->type = t_bool;
  p->ival = value;
  p->lineno = lineno;
	
  return p;
}

node* mkchar(char c)
{
  node* p;
  size_t size = sizeof(struct nodeTag) + sizeof(char);
		
  if ((p = (struct nodeTag *)malloc(size)) == NULL)
    memory_alloc_error();

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
  char* temp = (char*)malloc(destlen + 1);
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

node *mksym(char* s)
{
  node *p;
  size_t size = sizeof(struct nodeTag) + strlen(s) + 1;

  if ((p = (struct nodeTag *)malloc(size)) == NULL)
    memory_alloc_error();
  
  p->type = t_symbol;
  p->sval = strdup(s);
  p->lineno = lineno;
	
  return p;
}

node *mkcons(int oper, int nops, ...)
{
  node *p;  
  size_t size = sizeof(struct nodeTag) + sizeof(struct oprNodeType);

  if ((p = (struct nodeTag *)malloc(size)) == NULL)
    memory_alloc_error();
	
  p->type = t_cons;
  p->lineno = lineno;
  p->opr.oper = oper;
  p->opr.nops = nops;
  p->env = NULL;

  va_list ap;
  va_start(ap, nops);
	
  for (int i = 0; i < nops; i++)
    {
      node *arg = va_arg(ap, node*);
      p->opr.op[i] = (struct nodeTag *)malloc(sizeof(arg));
      p->opr.op[i] = arg;
    }
	
  va_end(ap);
	
  return p;
}

void memory_alloc_error()
{
  printf("memory_alloc_error()\n");
  abort();
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
	
  free(p);
}

node* car(node* node)
{
  if (node->opr.nops > 0)
    return node->opr.op[0];
  else
    return NODE_EMPTY;
}

node* cdr(node* node)
{
  switch (node->opr.nops)
    {
    case 0:
    case 1:
      return NODE_EMPTY;
    case 2:
      return node->opr.op[1];
    }
}

bool empty(node *list)
{
  return list->type == t_cons && list->opr.nops == 0;
}

node* append(node* list1, node* list2)
{
  if (list1->type != t_cons)
    list1 = mkcons(LIST, 1, list1);

  if (list2->type != t_cons)
    list2 = mkcons(LIST, 1, list2);

  if (empty(list2))
    return list1;

  if (empty(list1))
    return list2;

  node* r = list1;
  node* n = list1;
	
  while(n->opr.op[1] != NULL && !empty(n->opr.op[1]))
    {
      n = n->opr.op[1];
    }
	
  n->opr.nops = 2;
  n->opr.op[1] = list2;
	
  return r;
}

node *range(node *s, node *e)
{
  int from = s->ival;
  int to = e->ival;
  node *list = NODE_EMPTY;

  for (int i = to; i >= from; --i)
    list = append(mkint(i), list);

  return list;
}

int node_type(node* node)
{
  return node->type;
}

void display_primitive(node* node)
{
  switch (node->type)
    {
    case t_int:
      printf("%i", node->ival);
      break;
    case t_float:
      printf("%g", node->fval);
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
              printf("\"");

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

              printf("\"");
              break;
            }
		
          case LIST:
            {
              printf("[");
              
              while (node != NULL)
                {
                  switch (node->opr.nops)
                    {
                    case 0:
                      node = NULL;
                      break;
                    case 1:
                      display_primitive(node->opr.op[0]);
                      node = NULL;
                      break;
                    case 2:
                      display_primitive(node->opr.op[0]);
                      printf(",");       
                      node = node->opr.op[1];
                      break;
                    }
                }
					
              printf("]");
              break;
            }
		
          case APPLY:
            {
              display_primitive(node->opr.op[0]);
              printf("(");
              display_primitive(node->opr.op[1]);
              printf(")");
              break;
            }
				
          case LAMBDA:
            {
              printf("fn (");
              display_primitive(node->opr.op[0]);
              printf(")\n");
              display_primitive(node->opr.op[1]);
              printf("end\n");
              break;
            }

          case ';':
            {
              display_primitive(node->opr.op[0]);
              printf("\n");
              display_primitive(node->opr.op[1]);
              break;
            }

          case ',':
            {
              struct nodeTag *params = node;

              while (params != NULL)
                {
                  if (params->opr.nops > 0)
                    {
                      display_primitive(params->opr.op[0]);
                      params = params->opr.op[1];
                      if (params != NULL)
                        printf(", ");
                    }
                  else
                    params = NULL;
                }

              break;
            }

          case DEF:
            {
              display_primitive(node->opr.op[0]);
              printf(" = ");
              display_primitive(node->opr.op[1]);
              break;
            }

          case IF:
            {
              printf("if ");
              display_primitive(node->opr.op[0]);
              printf(" then\n");
              display_primitive(node->opr.op[1]);
              printf("\nend\n");
              break;
            }

          case '>':
          case '<':
          case '+':
          case '-':
          case '*':
          case '/':
          case GE:
          case LE:
          case MOD:
          case AND:
          case OR:
          case EQ:
          case NE:
            {
              display_primitive(node->opr.op[0]);
              
              switch (node->opr.oper)
                {
                case '>': printf(" > "); break;
                case '<': printf(" < "); break;
                case '+': printf(" + "); break;
                case '-': printf(" - "); break;
                case '*': printf(" * "); break;
                case '/': printf(" / "); break;
                case GE: printf(" >= "); break;
                case LE: printf(" <= "); break;
                case MOD: printf(" mod "); break;
                case AND: printf(" and "); break;
                case OR: printf(" or "); break;
                case EQ: printf(" == "); break;
                case NE: printf(" != "); break;
                }

              display_primitive(node->opr.op[1]);
              break;
            }
          }
			
        break;
      }
		
    case t_symbol:
      printf("%s", node->sval);
      break;
    }		
}

void display(node* node)
{
  if (node == NULL)
    return;
		
  display_primitive(node);
		
  printf("\n");
}

#define IS_NUMERIC_TYPE(x) (x->type == t_int || x->type == t_float || x->type == t_char ? true : false)
#define EXTRACT_NUMBER(x) (x->type == t_float ? x->fval : x->ival)
#define NUMERIC_RETURN_TYPE(x, y) (x->type == t_float || y->type == t_float ? t_float : t_int)

node* add(node* x, node* y)
{
  if (!IS_NUMERIC_TYPE(x) || !IS_NUMERIC_TYPE(y))
    error("operands to operator + must be of a numeric type");

  switch (NUMERIC_RETURN_TYPE(x, y))
    {
    case t_int:
      return mkint(EXTRACT_NUMBER(x) + EXTRACT_NUMBER(y));
    case t_float:
      return mkfloat(EXTRACT_NUMBER(x) + EXTRACT_NUMBER(y));
    }
}

node* sub(node* x, node* y)
{
  if (!IS_NUMERIC_TYPE(x) || !IS_NUMERIC_TYPE(y))
    error("operands to operator - must be of a numeric type");

  switch (NUMERIC_RETURN_TYPE(x, y))
    {
    case t_int:
      return mkint(EXTRACT_NUMBER(x) - EXTRACT_NUMBER(y));
    case t_float:
      return mkfloat(EXTRACT_NUMBER(x) - EXTRACT_NUMBER(y));
    }
}

node* mul(node* x, node* y)
{
  if (!IS_NUMERIC_TYPE(x) || !IS_NUMERIC_TYPE(y))
    error("operands to operator * must be of a numeric type");

  switch (NUMERIC_RETURN_TYPE(x, y))
    {
    case t_int:
      return mkint(EXTRACT_NUMBER(x) * EXTRACT_NUMBER(y));
    case t_float:
      return mkfloat(EXTRACT_NUMBER(x) * EXTRACT_NUMBER(y));
    }
}

node* dvd(node* x, node* y)
{
  if (!IS_NUMERIC_TYPE(x) || !IS_NUMERIC_TYPE(y))
    error("operands to operator / must be of a numeric type");

  switch (NUMERIC_RETURN_TYPE(x, y))
    {
    case t_int:
      return mkint(EXTRACT_NUMBER(x) / EXTRACT_NUMBER(y));
    case t_float:
      return mkfloat(EXTRACT_NUMBER(x) / EXTRACT_NUMBER(y));
    }
}

node* lt(node* x, node* y)
{
  //if (!IS_NUMERIC_TYPE(x) || !IS_NUMERIC_TYPE(y))
  //  error("operands to operator < must be of a numeric type");

  return EXTRACT_NUMBER(x) < EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
}

node* gt(node* x, node* y)
{
  return not(lte(x, y));
}

node* lte(node* x, node* y)
{
  return EXTRACT_NUMBER(x) <= EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
}

node* gte(node* x, node* y)
{
  return not(lt(x, y));
}

node* list_eq(node* l1, node* l2)
{
  if (l1->opr.nops == 0 && l2->opr.nops == 0)
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
  if (x == NODE_BOOL_FALSE || y == NODE_BOOL_FALSE)
    return NODE_BOOL_FALSE;
  else
    return NODE_BOOL_TRUE;
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
    return 0;
}

node* library_load(char* name)
{
  char *libroot, libpath[500];
  libroot = getenv("PRIMER_LIBRARY_PATH");
  	
  if (libroot == NULL)
    {
      printf("The environment variable PRIMER_LIBRARY_PATH has not been set\n");
      exit(-1);
    }
    
  sprintf(libpath, "%s%s.pri", libroot, name);
    
  if (!file_exists(libpath))
    {
      printf("Unable to find library: %s\n", name);
      exit(-1);
    }
  
  return parsel(libpath);
}

void error(char *msg)
{
  printf("error: %s\n", msg);
  exit(0);
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
