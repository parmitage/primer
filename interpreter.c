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
	
  lineno = 1;
  parse(argv[1]);
  eval(ast, NULL);
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

    case t_symbol:
      {
        binding *b = environment_lookup(env, p->sval);
			
        if (b != NULL)
          eval(b->node, env);
        else
          error_log("unbound symbol", p);
      }

    case t_cons:
      {
        switch(p->opr.oper)
          {
          case PROG:
            {
              env = environment_new(NULL);
              eval(library_load("base"), env);
              eval(p->opr.op[0], env);
              env = environment_delete(env);
              break;
            }

          case USE:
            {
              char *name = p->opr.op[0]->sval;
              node *module = library_load(name);
              node *namespace = mkcons(LAMBDA, 2, NULL, NULL);
              namespace->env = environment_new(env);
              eval(module, namespace->env);
              binding* binding = binding_new(name, namespace);
              environment_extend(env, binding);
              break;
            }
				
          case DEF:
            {
              break;
            }
				
          case ASSIGN:
            {
              char* name = p->opr.op[0]->sval;
              binding* binding = binding_new(name, p->opr.op[1]);
              environment_extend(env, binding);
              break;
            }
				
          case LAMBDA:
            {
	      /* A functions has an associated environment in which its body
		 will be evaluated. The environment is an extension of the
		 defining environment which means that it captures all of the
		 currently defined bindings to form a closure. The catch is
		 that in the following call:
		 
		 f(f(x))
		 
		 the outer f creates an environment but this is immediately
		 overwritten by the inner f.
		 
		 To get around this problem we clone the function object itself
		 and assign it a unique environment before pushing it onto the
		 stack for FUNCALL to evaluate.
	      */
	      node *clone = mkcons(LAMBDA, 2, p->opr.op[0], p->opr.op[1]);
	      clone->env = environment_new(env);
	      push(clone);
              break;
            }
				
          case FUNCALL:
            {
              eval(p->opr.op[0], env);
              node* fn = pop();
              node *params = p->opr.op[1];
              bind(fn->opr.op[0], params, fn->env, env);
              p = fn->opr.op[1];
              env = fn->env;
              goto eval_start;
              break;
            }

          case MODULECALL:
            {
              node* module = NULL;              
              binding *b = environment_lookup(env, p->opr.op[0]->sval);
			
               if (b != NULL)
                 module = b->node;

              if (module != NULL)
                {
                  eval(p->opr.op[1], module->env);
                  node *fn = pop();
                  node *params = p->opr.op[2];
                  bind(fn->opr.op[0], params, fn->env, env);
                  p = fn->opr.op[1];
                  env = fn->env;
                  goto eval_start;
                }
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

          case COND:
            {
              eval(p->opr.op[0], env);
              node *pred = pop();
              bool pass = false;
		
              /* try the first branch, if that fails then try the elif's */
              if (pred->ival > 0)
                {
                  environment *ext = environment_new(env);
                  p = p->opr.op[1];
                  env = ext;
                  goto eval_start;
                }
              else
                {
                  pass = cond(p->opr.op[2], env, false);
                }

              /* if we've got this far, no branch has passed so if there's an
                 else clause, evaluate it */
              if (!pass && p->opr.nops == 4)
                {
                  environment *ext = environment_new(env);
                  p = p->opr.op[3];
                  env = ext;
                  goto eval_start;
                }
              
              break;
            }

          case LENGTH:
            {
              eval(p->opr.op[0], env);
              node* val = pop();
              push(mkint(length(val)));
              break;
            }

          case NTH:
            {
              eval(p->opr.op[0], env);
              node *list = pop();
              eval(p->opr.op[1], env);
              int index = pop()->ival, n = 0;
              bool found = false;

              while (!found)
                {
                  if (list == NULL || index < 0)
                    {
                      push(NODE_NIL);
                      found = true;
                    }
                  else if (index == n)
                    {
                      push(car(list));
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

bool cond(node *p, environment *env, bool match)
{
  bool m = match;

  if (match == false && p != NULL && p->opr.oper == ELIF)
    {	
      eval(p->opr.op[0], env);
      
      if (pop()->ival == true)
        {
          eval(p->opr.op[1], env);
          m = true;
        }
    }
  else if (match == false && p != NULL && p->opr.oper == '|')
    {
      if (p->opr.nops > 0)
        m = cond(p->opr.op[0], env, m);
      
      if (p->opr.nops > 1)
        m = cond(p->opr.op[1], env, m);
    }

  return m;
}

int length(node* node)
{
  if (node == NULL)
    return -1;

  if (node->type == t_cons)
    {
      if (node->opr.nops > 1 && node->opr.op[1] != NULL)
        return 1 + length(node->opr.op[1]);
      else
        return node->opr.nops;
    }
  else if (node->type == t_nil)
    return 0;
  else
    return 1;
}

node* bind(node *args, node *params, environment *fnenv, environment *argenv)
{
  if (params != NULL)
    {		
      if (params->opr.nops > 1)
        {
          bind(args->opr.op[1], params->opr.op[1], fnenv, argenv);
        }
		
      if (params->opr.nops > 0)
        {
          eval(params->opr.op[0], argenv);
          binding* binding = binding_new(args->opr.op[0]->sval, pop());
          environment_extend(fnenv, binding);
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
  env->bindings[env->count++] = binding;
}

binding* environment_lookup(environment* env, char* name)
{
  environment* top = env;

  while (env != NULL)
    {
      for (int i = 0; i < env->count; ++i)
	{
	  if (strcmp(env->bindings[i]->name, name) == 0)
	    {
	      environment_extend(top, env->bindings[i]);
	      return env->bindings[i];
	    }
	}

      env = env->enclosing;
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

node* mknil()
{
  node *p;
  size_t size = sizeof(struct nodeTag) + sizeof(int);
	
  if ((p = (struct nodeTag *)malloc(size)) == NULL)
    memory_alloc_error();
	
  p->type = t_nil;
  p->lineno = lineno;
	
  return p;
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
  if (nops == 0)
    return mknil();

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
  //printf("%i", depth);

  for (int i = 0; i < depth; ++i)
    printf(" ");

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
						
              display_primitive(node->opr.op[0], depth+2);
					
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
							
              break;
            }

          case FUNCALL:
            {
              display_primitive(node->opr.op[0], depth);
              printf("(");
              display_primitive(node->opr.op[1], 0);
              printf(")");
              break;
            }
				
          case LAMBDA:
            {
              printf("fn (");
              display_primitive(node->opr.op[0], depth);
              printf(")\n");
              display_primitive(node->opr.op[1], depth+2);
              printf("end\n");
              break;
            }

          case ';':
            {
              display_primitive(node->opr.op[0], 0);
              printf("\n");
              display_primitive(node->opr.op[1], depth);
              break;
            }

          case ',':
            {
              struct nodeTag *params = node;

              while (params != NULL)
                {
                  if (params->opr.nops > 0)
                    {
                      display_primitive(params->opr.op[0], 0);
                      params = params->opr.op[1];
                      if (params != NULL)
                        printf(", ");
                    }
                  else
                    params = NULL;
                }

              break;
            }

          case ASSIGN:
            {
              display_primitive(node->opr.op[0], depth);
              printf(" = ");
              display_primitive(node->opr.op[1], 0);
              break;
            }

          case IF:
            {
              printf("if ");
              display_primitive(node->opr.op[0], 0);
              printf(" then\n");
              display_primitive(node->opr.op[1], depth+2);
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
              display_primitive(node->opr.op[0], depth);
              
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

              display_primitive(node->opr.op[1], 0);
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

void error_log(char *msg, node *node)
{
  printf("error: %s\ndetails: ", msg);
  if (node != NULL)
    display(node);
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

