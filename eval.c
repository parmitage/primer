#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "utils.h"
#include "eval.h"
#include "y.tab.h"

static long cnt_alloc = 0;
static long cnt_free = 0;
static long cnt_inc = 0;
static long cnt_dec = 0;

int main(int argc, char** argv)
{
   defaults();

   if (args(argc, argv) == -1)
   {
      usage();
      return -1;
   }

   NODE_BOOL_TRUE = mkbool(true); NODE_BOOL_TRUE->rc = -1;
   NODE_BOOL_FALSE = mkbool(false); NODE_BOOL_FALSE->rc = -1;
	
   lineno = 1;
   parse(arg_fname);
   wildcard = intern("_");
   eval(ast, NULL);
   return 0;
}

void build_closure_env(node *n, env *fenv, env *cenv)
{
   /* The closure env is constructed by taking the global env
      and extending it with the bindings closed over by the lambda. */

   /* note that this function is not currently in use - the evaluator
      currently simply captures everything at the point the function
      is constructed, including things that are not needed... */

   switch (n->type)
   {
      case t_symbol:
      {
         /* not interested in globals as they're already included */
         binding *b = env_lookup(global, n->ival);

         if (b != NULL)
            return;

         b = env_lookup(fenv, n->ival);

         if (b != NULL)
         {
            env_extend(cenv, b);
            return;
         }

         break;
      }

      case t_cons:
      {
         for (int i = 0; i < n->opr.nops; ++i)
         {
            build_closure_env(n->opr.op[i], fenv, cenv);
         }

         break;
      }
   }
}

void env_rebase(env *e)
{
   env *parent = e->parent;
   env *grandparent;

   if (parent != NULL && parent->parent != NULL)
      grandparent = parent->parent;
   else
      return;

   /* copy any non-shadowed bindings from the parent into env */
   binding *b = parent->bind;
   
   while (b != NULL)
   {
      binding *b = env_lookup(e, b->node->ival);

      if (b == NULL)
         env_extend(e, b);

      b = b->prev;
   }

   /* rebase the env such that it's new parent is it's grandparent */
   e->parent = grandparent;
   
   /* free the parent env - should this call env_delete? if it does, what to
      do about the reference counts of the bindings that are now part of env? */
   free(parent);
}

env *tco_env;

node *eval(node *n, env *e)
{
   if (!n)
      return NODE_BOOL_FALSE;

  eval_start:

   switch (n->type)
   {
      case t_int:
      case t_float:
      case t_bool:
      case t_char:
      case t_closure:
         incref(n);
         return n;

      case t_symbol:
      {
         binding *b = env_lookup(e, n->ival);
			
         if (b != NULL)
            return eval(b->node, e);
         else
            error("unbound symbol");
      }

      case t_cons:
      {
         switch(n->opr.oper)
         {
            case PROG:
            {
               global = e = env_new(NULL);
               
               if (arg_loadlib == true)
                  eval(loadlib(arg_stdlib), e);

               eval(n->opr.op[0], e);
               env_delete(global);

               trace("inc = %ld\ndec = %ld\nalloc = %ld\nfree = %ld\n",
                           cnt_inc, cnt_dec, cnt_alloc, cnt_free);
               break;
            }

            case DEF:
            {
               symbol name = n->opr.op[0]->ival;
               binding* binding = binding_new(name, eval(n->opr.op[1], e));
               env_extend(e, binding);
               break;
            }
				
            case LAMBDA:
            {
               //env *ce = env_new(global);
               //build_closure_env(n->opr.op[1], env, ce);

               if (n->opr.nops == 2)
                  return mklambda(n->opr.op[0], n->opr.op[1], NULL, e);
               else
                  return mklambda(n->opr.op[0], n->opr.op[1], n->opr.op[2], e);
            }

            case APPLY:
            {
               symbol fsym = n->opr.op[0]->ival;
               node *fn = eval(n->opr.op[0], e);
               env *ext = env_new(fn->opr.env);

               /* parameters */
               node *args = n->opr.op[1];
               bind(fn->opr.op[0], args, ext, e);

               /* where clause */
               if (fn->opr.nops == 3)
                  eval(fn->opr.op[2], ext);

               //printf("%s: %i\n", symname(fsym),
               //  function_is_tail_recursive(fn->opr.op[1], fsym));

               /* function body */
               if (function_is_tail_recursive(fn->opr.op[1], fsym))
               {
                  if (tco_env != NULL)
                     env_delete(tco_env);
               
                  n = fn->opr.op[1];
                  e = tco_env = ext;
                  goto eval_start;
               }
               else
               {
                  node *ret = eval(fn->opr.op[1], ext);
                  //env = env_delete(ext);
                  return ret;
               }
            }

            case LIST:
            {
               /* because we can store symbols in lists we evaluate the contents */
               node *ret;
               
               switch (n->opr.nops)
               {
                  case 0:
                     ret = mkcons(LIST, 0);
                     break;
                  case 1:
                     ret = mkcons(LIST, 1, eval(n->opr.op[0], e));
                     break;
                  case 2:
                     ret = mkcons(LIST, 2, eval(n->opr.op[0], e), eval(n->opr.op[1], e));
                     break;
               }

               //decref(p);
               incref(ret);
               return ret;
            }
            
            case STRING:
            {
               node *ret;

               switch (n->opr.nops)
               {
                  case 0:
                     ret = mkcons(STRING, 0);
                     break;
                  case 1:
                     ret = mkcons(STRING, 1, eval(n->opr.op[0], e));
                     break;
                  case 2:
                     ret = mkcons(STRING, 2, eval(n->opr.op[0], e), eval(n->opr.op[1], e));
                     break;
               }

               //decref(p);
               incref(ret);
               return ret;
            }

            case SHOW:
            {
               node *val = eval(n->opr.op[0], e);
               display(val);
               return val;
            }
				
            case ';':
            {
               eval(n->opr.op[0], e);
               eval(n->opr.op[1], e);
               break;
            }

            case IF:
            {              			
               node *pred = eval(n->opr.op[0], e);

               if (pred->type != t_bool)
                  error("type of predicate is not boolean");
					
               if (pred->ival > 0)
                  n = n->opr.op[1];
               else
                  n = n->opr.op[2];

               goto eval_start;
            }

            case LENGTH:
            {
               node *val = eval(n->opr.op[0], e);
               return mkint(length(val));
            }

            case NTH:
            {
               node *list = eval(n->opr.op[0], e);
               int index = eval(n->opr.op[1], e)->ival, n = 0;
               bool found = false;

               while (!found)
               {
                  if (list == NULL || index < 0)
                  {
                     return mkcons(LIST, 0);
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
               return add(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case '-':
            {
               if (n->opr.nops == 1)
                  return sub(mkint(0), eval(n->opr.op[0], e));
               else if (n->opr.nops == 2)
                  return sub(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
            }
				
            case '*':
               return mul(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case '/':
               return dvd(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case '<':
               return lt(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case '>':
               return gt(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case GE:
               return gte(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case LE:
               return lte(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case NE:
               return neq(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case EQ:
               return eq(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case AND:
               return and(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case OR:
               return or(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case MOD:
               return mod(eval(n->opr.op[0], e), eval(n->opr.op[1], e));

            case APPEND:
               return append(eval(n->opr.op[0], e), eval(n->opr.op[1], e));

            case RANGE:
               return range(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case NOT:
               return not(eval(n->opr.op[0], e));
				
            case TYPE:
            {
               int t = n->opr.op[0]->type;
					
               if (t == t_symbol)
               {
                  binding *b = env_lookup(e, n->opr.op[0]->ival);
						
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

void bind(node *args, node *params, env *fnenv, env *argenv)
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
            env_extend(fnenv, b);
         }
         else if (args->opr.op[0]->type == t_cons && args->opr.op[0]->opr.oper == CONS)
            bindp(args->opr.op[0], n, fnenv);
      }
   }
}

void bindp(node *args, node *list, env *fnenv)
{
   node *head = car(list);
   node *rest = cdr(list);

   if (args->opr.op[0]->ival != wildcard)
   {
      binding *headb = binding_new(args->opr.op[0]->ival, head);
      env_extend(fnenv, headb);
   }

   if (args->opr.op[1]->type == t_symbol)
   {
      if (args->opr.op[1]->ival != wildcard)
      {
         binding *restb = binding_new(args->opr.op[1]->ival, rest);
         env_extend(fnenv, restb);
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

env *env_new(env *parent)
{
   size_t sz = sizeof(env);
   env *e = (env *) malloc(sz);
   e->parent = parent;
   e->bind = NULL;
   return e;
}

void incref(node *n)
{
   if (!refctr)
      return;

   if (n->rc == -1)
      return;

   n->rc++;

   /* DEBUG */
   cnt_inc++;

   //printf("incref to %i for ", n->rc);
   //display(n);
}

void decref(node *n)
{
   if (!refctr)
      return;

   if (n == NULL)
      return;

   if (n->rc == -1)
      return;

   n->rc--;

   /* if (n->type == t_cons && n->opr.oper == LIST) */
   /* { */
   /*    for (int i = 0; i < n->opr.nops; i++) */
   /*       decref(n->opr.op[i]); */
   /* } */

   /* DEBUG */
   cnt_dec++;

   //printf("decref to %i for ", n->rc);
   //display(n);

   if (n->rc == 0)
   {
      /* DEBUG */
      cnt_free++;

      free(n);
   }
}

env *env_delete(env *e)
{
   env *parent = e->parent;
   binding *b = e->bind;

   if (refctr)
   {
      while (b != NULL)
      {
         node *n = b->node;
         decref(n);
         binding *temp = b;
         b = b->prev;
         free(temp);
      }
      
      free(e);
   }

   return parent;
}

void env_extend(env *e, binding *nb)
{
   symbol s = nb->sym;
   binding *h = e->bind;
   
   while (h != NULL)
   {
      if (s == h->sym)
         error("symbol already bound in this env");

      h = h->prev;
   }

   /* binding wasn't found so create a new one */
   nb->prev = e->bind;
   e->bind = nb;
}

binding* env_lookup(env *e, symbol sym)
{
   env *top = e;
   bool depth = false;

   while (e != NULL)
   {
      binding *b = e->bind;

      while (b != NULL)
      {
         if (sym == b->sym)
         {
            /* lift a binding into this env */
            /* TODO need to clone the env in this new design... */
            /* if (depth) */
            /* { */
            /*    env_extend(top, b); */
            /* } */
            return b;
         }

         b = b->prev;
      }

      e = e->parent;
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

struct node *prialloc(size_t sz)
{
   cnt_alloc++;
   return (node *) malloc(sz);
}

node *mkint(int value)
{
   node *p;
   int size = sizeof(struct node) + sizeof(int);

   if ((p = (struct node*)prialloc(size)) == NULL)
      memory_alloc_error();
	
   p->type = t_int;
   p->ival = value;
   p->lineno = lineno;
   p->rc = 1;
	
   return p;
}

node *mkfloat(float value)
{
   node *p;
   size_t size = sizeof(struct node) + sizeof(float);
	
   if ((p = (struct node*)prialloc(size)) == NULL)
      memory_alloc_error();
  
   p->type = t_float;
   p->fval = value;
   p->lineno = lineno;
   p->rc = 1;
	
   return p;
}

node *mkbool(int value)
{
   node *p;
   size_t size = sizeof(struct node) + sizeof(int);
	
   if ((p = (struct node*)prialloc(size)) == NULL)
      memory_alloc_error();
	
   p->type = t_bool;
   p->ival = value;
   p->lineno = lineno;
   p->rc = 1;

   return p;
}

node* mkchar(char c)
{
   node* p;
   size_t size = sizeof(struct node) + sizeof(char);
		
   if ((p = (struct node*)prialloc(size)) == NULL)
      memory_alloc_error();

   p->type = t_char;
   p->lineno = lineno;
   p->ival = c;
   p->rc = 1;
	
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

   if ((p = (struct node*)prialloc(size)) == NULL)
      memory_alloc_error();
  
   p->type = t_symbol;
   p->ival = intern(s);
   p->lineno = lineno;
   p->rc = -1;
	
   return p;
}

node *mkcons(int oper, int nops, ...)
{
   node *p;  
   size_t size = sizeof(struct node) + sizeof(struct cons);

   if ((p = (struct node*)prialloc(size)) == NULL)
      memory_alloc_error();
	
   p->type = t_cons;
   p->lineno = lineno;
   p->rc = 1;
   p->opr.oper = oper;
   p->opr.nops = nops;
   p->opr.env = NULL;

   va_list ap;
   va_start(ap, nops);
	
   for (int i = 0; i < nops; i++)
   {
      node *arg = va_arg(ap, node*);
      p->opr.op[i] = (struct node*)prialloc(sizeof(arg));
      p->opr.op[i] = arg;
   }
	
   va_end(ap);
	
   return p;
}

node *mklambda(node *params, node *body, node *where, env *e)
{
   node *p;  
   size_t size = sizeof(struct node) + sizeof(struct cons);

   if ((p = (struct node*)prialloc(size)) == NULL)
      memory_alloc_error();
	
   p->type = t_closure;
   p->lineno = lineno;
   p->rc = 1;
   p->opr.oper = LAMBDA;
   p->opr.nops = where == NULL ? 2 : 3;
   p->opr.env = env_new(e);
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

node *car(node *node)
{
   struct node *ret;

   if (node->opr.nops > 0)
      ret = node->opr.op[0];
   else
      ret = mkcons(LIST, 0);

   decref(node);
   return ret;
}

node *cdr(node *node)
{
   struct node *ret;

   switch (node->opr.nops)
   {
      case 0:
      case 1:
         ret = mkcons(LIST, 0);
         break;
      case 2:
         ret = node->opr.op[1];
         break;
   }

   decref(node);
   return ret;
}

bool empty(node *list)
{
   return list->type == t_cons && list->opr.nops == 0;
}

int length(node *node)
{
   if (node->type != t_cons)
      error("argument to length must be a list");

   int len = 0;
   struct node *iter = node;

   while (iter != NULL && iter->opr.nops > 0)
   {
      len += 1;
      iter = iter->opr.op[1];
   }

   //printf("length=%i and rc=%i\n", len, node->rc);
   decref(node);
   return len;
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
   node *list = mkcons(LIST, 0);

   for (int i = to; i >= from; --i)
      list = append(mkint(i), list);

   decref(s);
   decref(e);
   return list;
}

void pprint(node *node)
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
		    
		if (node->opr.op[2] != NULL)
		{
			printf("where ");
			pprint(node->opr.op[2]);
		}
		    
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
         printf("%s", symname(node->ival));
         break;
      
      case t_closure:
        {	
	       printf("fn (");
	       pprint(node->opr.op[0]);
	       printf(")");
	       pprint(node->opr.op[1]);
		
		if (node->opr.op[2] != NULL)
		{
			printf("\twhere ");
			pprint(node->opr.op[2]);
			printf("\n");
		}
		
	       printf("end");
	       break;
	}
   }		
}

void display(node* n)
{
   if (n == NULL)
      return;
		
   pprint(n);
   printf("\n");
}

#define IS_NUMERIC_TYPE(x) (x->type == t_int || x->type == t_float || x->type == t_char ? true : false)
#define EXTRACT_NUMBER(x) (x->type == t_float ? x->fval : x->ival)
#define NUMERIC_RETURN_TYPE(x, y) (x->type == t_float || y->type == t_float ? t_float : t_int)

node *add(node *x, node *y)
{
   if (!IS_NUMERIC_TYPE(x) || !IS_NUMERIC_TYPE(y))
      error("operands to operator + must be of a numeric type");

   node *ret;

   switch (NUMERIC_RETURN_TYPE(x, y))
   {
      case t_int:
         ret = mkint(EXTRACT_NUMBER(x) + EXTRACT_NUMBER(y));
         break;
      case t_float:
         ret = mkfloat(EXTRACT_NUMBER(x) + EXTRACT_NUMBER(y));
         break;
   }

   decref(x);
   decref(y);
   return ret;
}

node *sub(node *x, node *y)
{
   if (!IS_NUMERIC_TYPE(x) || !IS_NUMERIC_TYPE(y))
      error("operands to operator - must be of a numeric type");

   node *ret;

   switch (NUMERIC_RETURN_TYPE(x, y))
   {
      case t_int:
         ret = mkint(EXTRACT_NUMBER(x) - EXTRACT_NUMBER(y));
         break;
      case t_float:
         ret = mkfloat(EXTRACT_NUMBER(x) - EXTRACT_NUMBER(y));
         break;
   }

   decref(x);
   decref(y);
   return ret;
}

node *mul(node *x, node *y)
{
   if (!IS_NUMERIC_TYPE(x) || !IS_NUMERIC_TYPE(y))
      error("operands to operator * must be of a numeric type");

   node *ret;

   switch (NUMERIC_RETURN_TYPE(x, y))
   {
      case t_int:
         ret = mkint(EXTRACT_NUMBER(x) * EXTRACT_NUMBER(y));
         break;
      case t_float:
         ret = mkfloat(EXTRACT_NUMBER(x) * EXTRACT_NUMBER(y));
         break;
   }

   decref(x);
   decref(y);
   return ret;
}

node *dvd(node *x, node *y)
{
   if (!IS_NUMERIC_TYPE(x) || !IS_NUMERIC_TYPE(y))
      error("operands to operator / must be of a numeric type");

   node *ret;

   switch (NUMERIC_RETURN_TYPE(x, y))
   {
      case t_int:
         ret = mkint(EXTRACT_NUMBER(x) / EXTRACT_NUMBER(y));
         break;
      case t_float:
         ret = mkfloat(EXTRACT_NUMBER(x) / EXTRACT_NUMBER(y));
         break;
   }

   decref(x);
   decref(y);
   return ret;
}

node *lt(node *x, node *y)
{
   node *ret = EXTRACT_NUMBER(x) < EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;

   decref(x);
   decref(y);
   return ret;
}

node *gt(node *x, node *y)
{
   return not(lte(x, y));
}

node *lte(node *x, node *y)
{
   node *ret = EXTRACT_NUMBER(x) <= EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
   
   decref(x);
   decref(y);
   return ret;
}

node *gte(node *x, node *y)
{
   return not(lt(x, y));
}

node *list_eq(node *l1, node *l2)
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

node *eq(node *x, node *y)
{
   node *ret;

   if (x->type != y->type)
      ret = NODE_BOOL_FALSE;
   else if (x->type == t_int || x->type == t_bool)
      ret = x->ival == y->ival ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
   else if (x->type == t_float)
      ret = x->fval == y->fval ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
   else if (x->type == t_char)
      ret = x->ival == y->ival ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
   else if (x->type == t_cons)
      ret = list_eq(x, y);
   else
      ret = NODE_BOOL_FALSE;

   decref(x);
   decref(y);
   return ret;
}

node *neq(node *x, node *y)
{
   return(not(eq(x, y)));
}

node *and(node *x, node *y)
{
   if (x->type != t_bool)
      error("left operand to operator and must be boolean");

   if (y->type != t_bool)
      error("right operand to operator and must be boolean");

   node *ret;

   if (x == NODE_BOOL_FALSE || y == NODE_BOOL_FALSE)
      ret = NODE_BOOL_FALSE;
   else
      ret = NODE_BOOL_TRUE;

   decref(x);
   decref(y);
   return ret;
}

node *or(node *x, node *y)
{
   if (x->type != t_bool)
      error("left operand to operator or must be boolean");

   if (y->type != t_bool)
      error("right operand to operator or must be boolean");
  
   node *ret;

   if (x->ival || y->ival)
      ret = NODE_BOOL_TRUE;
   else
      ret = NODE_BOOL_FALSE;

   decref(x);
   decref(y);
   return ret;
}

node *not(node *x)
{
   if (x->type != t_bool)
      error("operand to operator not must be boolean");

   node *ret;

   if (x->ival == true)
      ret = NODE_BOOL_FALSE;
   else
      ret = NODE_BOOL_TRUE;

   decref(x);
   return ret;
}

node *mod(node *x, node *y)
{
   node *retval;

   if (x->type == t_int && y->type == t_int)
      retval = mkint(x->ival % y->ival);
   else
      error("operator mod can only be applies to integer operands");

   decref(x);
   decref(y);
   return retval;
}

node *loadlib(char *name)
{
   char *libroot, libpath[500];
   libroot = getenv("PRIMER_LIBRARY_PATH");
  	
   if (libroot == NULL)
   {
      printf("The environment variable PRIMER_LIBRARY_PATH has not been set\n");
      exit(-1);
   }
    
   sprintf(libpath, "%s%s.pri", libroot, name);
    
   if (!fexists(libpath))
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

symbol intern(char *string)
{
   static int new_symbol_index = 0;

   for (int i = 0; i < new_symbol_index; i++)
   {
      if (strcmp(string, symtab[i]) == 0)
         return i;
   }
  
   symtab[new_symbol_index] = string;
   return new_symbol_index++;
}

char *symname(symbol s)
{
   return symtab[s];
}
