#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "eval.h"
#include "args.h"
#include "y.tab.h"

int main(int argc, char** argv)
{
   defaults();

   if (getargs(argc, argv) == -1)
   {
      usage();
      return -1;
   }

   NODE_EMPTY = mkcons(LIST, 0);
   NODE_BOOL_TRUE = mkbool(true);
   NODE_BOOL_FALSE = mkbool(false);
   NODE_INT_ZERO = mkint(0);
	
   lineno = 1;
   parse(fname);
   wildcard = intern("_");
   eval(ast, NULL);
   return 0;
}

void build_closure_environment(node *n, environment *fenv, environment *cenv)
{
   /* The closure environment is constructed by taking the global environment
      and extending it with the bindings closed over by the lambda. */

   switch (n->type)
   {
      case t_symbol:
      {
         /* not interested in globals as they're already included */
         binding *b = environment_lookup(global, n->ival);

         if (b != NULL)
            return;

         b = environment_lookup(fenv, n->ival);

         if (b != NULL)
         {
            environment_extend(cenv, b);
            return;
         }

         break;
      }

      case t_cons:
      {
         for (int i = 0; i < n->opr.nops; ++i)
         {
            build_closure_environment(n->opr.op[i], fenv, cenv);
         }

         break;
      }
   }
}

node *eval(node *p, environment *env)
{
   if (!p)
      return NODE_BOOL_FALSE;

  eval_start:

   switch (p->type)
   {
      case t_int:
      case t_float:
      case t_bool:
      case t_char:
      case t_closure:
         return p;

      case t_symbol:
      {
         binding *b = environment_lookup(env, p->ival);
			
         if (b != NULL)
            return eval(b->node, env);
         else
         {
            error("unbound symbol");
         }
      }

      case t_cons:
      {
         switch(p->opr.oper)
         {
            case PROG:
            {
               global = env = environment_new(NULL);
               
               if (loadlib == true)
                  eval(library_load(stdlib), env);

               eval(p->opr.op[0], env);
               break;
            }

            case DEF:
            {
               symbol name = p->opr.op[0]->ival;
               binding* binding = binding_new(name, eval(p->opr.op[1], env));
               environment_extend(env, binding);
               break;
            }
				
            case LAMBDA:
            {
               //environment *ce = environment_new(global);
               //build_closure_environment(p->opr.op[1], env, ce);

               if (p->opr.nops == 2)
                  return mklambda(p->opr.op[0], p->opr.op[1], NULL, env);
               else
                  return mklambda(p->opr.op[0], p->opr.op[1], p->opr.op[2], env);
            }

            case APPLY:
            {
               symbol fsym = p->opr.op[0]->ival;
               node *fn = eval(p->opr.op[0], env);
               environment *ext = environment_new(fn->opr.env);

               /* parameters */
               node *args = p->opr.op[1];
               bind(fn->opr.op[0], args, ext, env);

               /* where clause */
               if (fn->opr.nops == 3)
                  eval(fn->opr.op[2], ext);

               //printf("%s: %i\n", symbol_name(fsym),
               //  function_is_tail_recursive(fn->opr.op[1], fsym));

               /* function body */
               if (function_is_tail_recursive(fn->opr.op[1], fsym))
               {
                  environment_delete(env);
                  p = fn->opr.op[1];
                  env = ext;
                  goto eval_start;
               }
               else
               {
                  node *ret = eval(fn->opr.op[1], ext);
                  env = environment_delete(ext);
                  return ret;
               }
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
                  binding *b = environment_lookup(env, p->opr.op[0]->ival);
						
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
   if (node->type != t_cons)
      error("argument to length must be a list");

   int len = 0;

   while (node != NULL && node->opr.nops > 0)
   {
      len += 1;
      node = node->opr.op[1];
   }

   return len;
}

void bind(node *args, node *params, environment *fnenv, environment *argenv)
{
   if (params != NULL)
   {		
      if (params->opr.nops > 1)
         bind(args->opr.op[1], params->opr.op[1], fnenv, argenv);
		
      if (params->opr.nops > 0)
      {
         node *n = eval(params->opr.op[0], argenv);
          
         if (args->opr.op[0]->type == t_symbol)
         {
            binding* b = binding_new(args->opr.op[0]->ival, n);
            environment_extend(fnenv, b);
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

   if (args->opr.op[0]->ival != wildcard)
   {
      binding *headb = binding_new(args->opr.op[0]->ival, head);
      environment_extend(fnenv, headb);
   }

   if (args->opr.op[1]->type == t_symbol)
   {
      if (args->opr.op[1]->ival != wildcard)
      {
         binding *restb = binding_new(args->opr.op[1]->ival, rest);
         environment_extend(fnenv, restb);
      }
   }
   else
      bindp(args->opr.op[1], rest, fnenv);
}

binding* binding_new(symbol s, node *n)
{
   size_t sz = sizeof(binding);
   binding *b = (binding *) malloc(sz);	
   b->sym = s;
   b->node = n;
   b->prev = NULL;
   return b;
}

environment* environment_new(environment *parent)
{
   size_t sz = sizeof(environment);
   environment *env = (environment *) malloc(sz);
   env->enclosing = parent;
   env->bind = NULL;
   return env;
}

environment *environment_delete(environment *env)
{
   environment *parent = env->enclosing;

   if (env->enclosing != NULL)
   {

   }
   
   return parent;
}

void environment_extend(environment *env, binding *nb)
{
   symbol s = nb->sym;
   binding *h = env->bind;
   
   while (h != NULL)
   {
      if (s == h->sym)
         error("symbol already bound in this environment");

      h = h->prev;
   }

   /* binding wasn't found so create a new one */
   nb->prev = env->bind;
   env->bind = nb;
}

binding* environment_lookup(environment *env, symbol sym)
{
   environment *top = env;
   bool depth = false;

   while (env != NULL)
   {
      binding *b = env->bind;

      while (b != NULL)
      {
         if (sym == b->sym)
         {
            /* lift a binding into this environment */
            /* TODO need to clone the environment in this new design... */
            /* if (depth) */
            /* { */
            /*    environment_extend(top, b); */
            /* } */

            return b;
         }

         b = b->prev;
      }

      env = env->enclosing;
      depth = true;
   }
  
   return NULL;
}

bool function_is_tail_recursive(node *expr, symbol s)
{
/* TODO more cases that need special handling... */
   
   switch (expr->type)
   {
      case t_int:
      case t_float:
      case t_bool:
      case t_char:
      case t_symbol:
         return true;
      case t_cons:
         switch(expr->opr.oper)
         {
            case APPLY:
               return expr->opr.op[0]->ival == s;
            case IF:
               return function_is_tail_recursive(expr->opr.op[1], s) &&
                  function_is_tail_recursive(expr->opr.op[2], s);
            default:
               return false;
         }
      default:
         return false;
   }
}

node *mkint(int value)
{
   node *p;
   int size = sizeof(struct node) + sizeof(int);

   if ((p = (struct node*)malloc(size)) == NULL)
      memory_alloc_error();
	
   p->type = t_int;
   p->ival = value;
   p->lineno = lineno;
	
   return p;
}

node *mkfloat(float value)
{
   node *p;
   size_t size = sizeof(struct node) + sizeof(float);
	
   if ((p = (struct node*)malloc(size)) == NULL)
      memory_alloc_error();
  
   p->type = t_float;
   p->fval = value;
   p->lineno = lineno;
	
   return p;
}

node *mkbool(int value)
{
   node *p;
   size_t size = sizeof(struct node) + sizeof(int);
	
   if ((p = (struct node*)malloc(size)) == NULL)
      memory_alloc_error();
	
   p->type = t_bool;
   p->ival = value;
   p->lineno = lineno;
	
   return p;
}

node* mkchar(char c)
{
   node* p;
   size_t size = sizeof(struct node) + sizeof(char);
		
   if ((p = (struct node*)malloc(size)) == NULL)
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
   size_t size = sizeof(struct node) + sizeof(int);

   if ((p = (struct node*)malloc(size)) == NULL)
      memory_alloc_error();
  
   p->type = t_symbol;
   p->ival = intern(s);
   p->lineno = lineno;
	
   return p;
}

node *mkcons(int oper, int nops, ...)
{
   node *p;  
   size_t size = sizeof(struct node) + sizeof(struct cons);

   if ((p = (struct node*)malloc(size)) == NULL)
      memory_alloc_error();
	
   p->type = t_cons;
   p->lineno = lineno;
   p->opr.oper = oper;
   p->opr.nops = nops;
   p->opr.env = NULL;

   va_list ap;
   va_start(ap, nops);
	
   for (int i = 0; i < nops; i++)
   {
      node *arg = va_arg(ap, node*);
      p->opr.op[i] = (struct node*)malloc(sizeof(arg));
      p->opr.op[i] = arg;
   }
	
   va_end(ap);
	
   return p;
}

node *mklambda(node *params, node *body, node *where, environment *e)
{
   node *p;  
   size_t size = sizeof(struct node) + sizeof(struct cons);

   if ((p = (struct node*)malloc(size)) == NULL)
      memory_alloc_error();
	
   p->type = t_closure;
   p->lineno = lineno;
   p->opr.oper = LAMBDA;
   p->opr.nops = where == NULL ? 2 : 3;
   p->opr.env = environment_new(e);
   p->opr.op[0] = params;
   p->opr.op[1] = body;
   p->opr.op[2] = where;

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

node *car(node *node)
{
   if (node->opr.nops > 0)
      return node->opr.op[0];
   else
      return NODE_EMPTY;
}

node *cdr(node *node)
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

node *append(node *list1, node *list2)
{
   if (list1->type != t_cons)
      list1 = mkcons(LIST, 1, list1);

   if (list2->type != t_cons)
      list2 = mkcons(LIST, 1, list2);

   if (empty(list2))
      return list1;

   if (empty(list1))
      return list2;

   node *r = list1;
   node *n = list1;
	
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

void pprint(node* node)
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
                        pprint(node->opr.op[0]);
                        node = NULL;
                        break;
                     case 2:
                        pprint(node->opr.op[0]);
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
               pprint(node->opr.op[0]);
               printf("(");
               pprint(node->opr.op[1]);
               printf(")");
               break;
            }
				
            case LAMBDA:
            {
               printf("\n\tfn (");
               pprint(node->opr.op[0]);
               printf(") ");
               pprint(node->opr.op[1]);
               printf(" end\n");
               break;
            }

            case ';':
            {
               pprint(node->opr.op[0]);
               printf("\n");
               pprint(node->opr.op[1]);
               break;
            }

            case ',':
            {
               struct node *params = node;

               while (params != NULL)
               {
                  if (params->opr.nops > 0)
                  {
                     pprint(params->opr.op[0]);
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
               pprint(node->opr.op[0]);
               printf(" = ");
               pprint(node->opr.op[1]);
               break;
            }

            case IF:
            {
               printf("if ");
               pprint(node->opr.op[0]);
               printf(" then ");
               pprint(node->opr.op[1]);
               printf("\nelse ");
               pprint(node->opr.op[2]);
               break;
            }

            case CONS:
            {
               pprint(node->opr.op[0]);
               printf(":");
               pprint(node->opr.op[1]);
               break;
            }

            case SHOW:
            {
               printf("show(");
               pprint(node->opr.op[0]);
               printf(")");
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
            case APPEND:
            {
               pprint(node->opr.op[0]);
              
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
                  case APPEND: printf(" ++ "); break;
               }

               pprint(node->opr.op[1]);
               break;
            }
         }
			
         break;
      }
		
      case t_symbol:
         printf("%s", symbol_name(node->ival));
         break;
      
      case t_closure:
        {
	       printf("fn (");
	       pprint(node->opr.op[0]);
	       printf(")");
	       pprint(node->opr.op[1]);
	       printf("end");
	       break;
	}
   }		
}

void display(node* node)
{
   if (node == NULL)
      return;
		
   pprint(node);
		
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
   if (x->type != t_bool)
      error("left operand to operator and must be boolean");

   if (y->type != t_bool)
      error("right operand to operator and must be boolean");

   if (x == NODE_BOOL_FALSE || y == NODE_BOOL_FALSE)
      return NODE_BOOL_FALSE;
   else
      return NODE_BOOL_TRUE;
}

node* or(node* x, node* y)
{
   if (x->type != t_bool)
      error("left operand to operator or must be boolean");

   if (y->type != t_bool)
      error("right operand to operator or must be boolean");
  
   if (x->ival || y->ival)
      return NODE_BOOL_TRUE;
   else
      return NODE_BOOL_FALSE;
}

node* not(node* x)
{
   if (x->type != t_bool)
      error("operand to operator not must be boolean");

   if (x->ival == true)
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

symbol intern(char *string)
{
   static int new_symbol_index = 0;

   for (int i = 0; i < new_symbol_index; i++)
   {
      if (strcmp(string, symbol_table[i]) == 0)
         return i;
   }
  
   symbol_table[new_symbol_index] = string;
   return new_symbol_index++;
}

char *symbol_name(symbol s)
{
   return symbol_table[s];
}
