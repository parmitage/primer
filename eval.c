#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "types.h"
#include "utils.h"
#include "eval.h"
#include "y.tab.h"

static long cnt_alloc = 0;
static long cnt_free = 0;
static long cnt_inc = 0;
static long cnt_dec = 0;

int main(int argc, char **argv)
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
         return n;

      case t_symbol:
      {
         binding *b = envlookup(e, n->ival);
			
         if (b != NULL)
         {
            incref(b->node);
            return eval(b->node, e);
         }
         else
            error("unbound symbol");
      }

      case t_tuple:
      {
         node *tuple = mktuple();
         
         for (int i = 0; i < n->tuple.count; ++i)
            tuple->tuple.n[i] = eval(n->tuple.n[i], e);

         tuple->tuple.count = n->tuple.count;

         return tuple;
      }

      case t_pair:
      {
         switch(n->opr.oper)
         {
            case PROG:
            {
               global = e = envnew(NULL);
               
               if (arg_loadlib == true)
                  eval(loadlib(arg_stdlib), e);

               eval(n->opr.op[0], e);
               envdel(global);

               trace("[inc=%ld, dec=%ld, alloc=%ld, free=%ld]",
                     cnt_inc, cnt_dec, cnt_alloc, cnt_free);
               break;
            }

            case DEF:
            {
               symbol name = n->opr.op[0]->ival;
               binding *binding = bindnew(name, eval(n->opr.op[1], e));
               envext(e, binding);
               break;
            }
				
            case LAMBDA:
            {
               //env *ce = envnew(global);
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
               env *ext = envnew(fn->opr.env);

               /* parameters */
               node *args = n->opr.op[1];
               bind(fn->opr.op[0], args, ext, e);

               /* where clause */
               if (fn->opr.nops == 3)
                  eval(fn->opr.op[2], ext);

               /* function body */
               if (istailrecur(fn->opr.op[1], fsym))
               {
                  if (tco_env != NULL)
                     envdel(tco_env);
               
                  n = fn->opr.op[1];
                  e = tco_env = ext;
                  goto eval_start;
               }
               else
               {
                  node *ret = eval(fn->opr.op[1], ext);
                  //e = envdel(ext);
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
                     ret = mkpair(LIST, 0);
                     break;
                  case 1:
                     ret = mkpair(LIST, 1, eval(n->opr.op[0], e));
                     break;
                  case 2:
                     ret = mkpair(LIST, 2, eval(n->opr.op[0], e), eval(n->opr.op[1], e));
                     break;
               }

               //decref(p);
               //incref(ret);
               return ret;
            }
           
            case STRING:
            {
               node *ret;

               switch (n->opr.nops)
               {
                  case 0:
                     ret = mkpair(STRING, 0);
                     break;
                  case 1:
                     ret = mkpair(STRING, 1, eval(n->opr.op[0], e));
                     break;
                  case 2:
                     ret = mkpair(STRING, 2, eval(n->opr.op[0], e), eval(n->opr.op[1], e));
                     break;
               }

               //decref(p);
               //incref(ret);
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
                     return mkpair(LIST, 0);
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

            case CONS:
               return cons(eval(n->opr.op[0], e), eval(n->opr.op[1], e));

            case RANGE:
               return range(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
				
            case NOT:
               return not(eval(n->opr.op[0], e));
				
            case TYPE:
            {
               int t = n->opr.op[0]->type;
					
               if (t == t_symbol)
               {
                  binding *b = envlookup(e, n->opr.op[0]->ival);
						
                  if (b != NULL)
                     t = b->node->type;
                  else
                     t = -1;
               }

               decref(n);
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
            binding* b = bindnew(args->opr.op[0]->ival, n);
            envext(fnenv, b);
         }
         else if (args->opr.op[0]->type == t_pair && args->opr.op[0]->opr.oper == CONS)
            bindp(args->opr.op[0], n, fnenv);
         else if (args->opr.op[0]->type == t_tuple)
            bindt(args->opr.op[0], n, fnenv);
      }
   }
}

void bindt(node *arg, node *tuple, env *fnenv)
{
   for (int i = 0; i < arg->tuple.count; ++i)
   {
      binding *b = bindnew(arg->tuple.n[i]->ival, tuple->tuple.n[i]);
      envext(fnenv, b);
   }
}

void bindp(node *args, node *list, env *fnenv)
{
   node *head = car(list);
   node *rest = cdr(list);

   if (args->opr.op[0]->ival != wildcard)
   {
      binding *headb = bindnew(args->opr.op[0]->ival, head);
      envext(fnenv, headb);
   }

   if (args->opr.op[1]->type == t_symbol)
   {
      if (args->opr.op[1]->ival != wildcard)
      {
         binding *restb = bindnew(args->opr.op[1]->ival, rest);
         envext(fnenv, restb);
      }
   }
   else
      bindp(args->opr.op[1], rest, fnenv);
}

binding* bindnew(symbol s, node *n)
{
   size_t sz = sizeof(binding);
   binding *b = (binding *) malloc(sz);	
   b->sym = s;
   b->node = n;
   b->prev = NULL;
   return b;
}

env *envnew(env *parent)
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

   /* DEBUG */
   cnt_dec++;

   //printf("decref to %i for ", n->rc);
   //display(n);

   if (n->rc == 0)
   {
      /* DEBUG */
      cnt_free++;

      if (n->type == t_pair && n->opr.oper == LIST)
      {
         for (int i = 0; i < n->opr.nops; i++)
            decref(n->opr.op[i]);
      }

      free(n);
   }
}

env *envdel(env *e)
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

void envext(env *e, binding *nb)
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

binding* envlookup(env *e, symbol sym)
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
            /*    envext(top, b); */
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

bool istailrecur(node *expr, symbol s)
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
      case t_pair:
         switch(expr->opr.oper)
         {
            case APPLY:
               return expr->opr.op[0]->ival == s;
            case IF:
               return istailrecur(expr->opr.op[1], s) &&
                  istailrecur(expr->opr.op[2], s);
            default:
               return false;
         }
      default:
         return false;
   }
}

struct node *prialloc()
{
   cnt_alloc++;
   node *p;

   if ((p = (struct node*)malloc(sizeof(struct node))) == NULL)
      error("Unable to allocate memory - terminating");

   return p;
}

node *mkint(int value)
{
   node *p = prialloc();
   p->type = t_int;
   p->ival = value;
   p->lineno = lineno;
   p->rc = 1;
   return p;
}

node *mkfloat(float value)
{
   node *p = prialloc();
   p->type = t_float;
   p->fval = value;
   p->lineno = lineno;
   p->rc = 1;
   return p;
}

node *mkbool(int value)
{
   node *p = prialloc();
   p->type = t_bool;
   p->ival = value;
   p->lineno = lineno;
   p->rc = 1;
   return p;
}

node* mkchar(char c)
{
   node *p = prialloc();
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
   return strtonode(temp);
}

node* strtonode(char* value)
{
   int len = strlen(value);

   if (len > 1)
      return mkpair(STRING, 2, mkchar(value[0]), strtonode(value + 1));
   else
      return mkpair(STRING, 1, mkchar(value[0]));
}

node *mksym(char* s)
{
   node *p = prialloc();
   p->type = t_symbol;
   p->ival = intern(s);
   p->lineno = lineno;
   p->rc = -1;
   return p;
}

node *mkpair(int oper, int nops, ...)
{
   node *p = prialloc();
   p->type = t_pair;
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
      p->opr.op[i] = arg;
   }
	
   va_end(ap);
	
   return p;
}

node *mktuple()
{
   node *p = prialloc();
   p->type = t_tuple;
   p->lineno = lineno;
   p->rc = 1;
   p->tuple.count = 0;
   return p;
}

node *list2tuple(node *list)
{
   node *tuple = mktuple();

   int len = 0;
   struct node *iter = list;

   while (iter != NULL && iter->opr.nops > 0)
   {
      tuple->tuple.n[len] = iter->opr.op[0];
      len++;      
      iter = iter->opr.op[1];
   }

   tuple->tuple.count = len;

   return tuple;
}

node *mklambda(node *params, node *body, node *where, env *e)
{
   node *p = prialloc();
   p->type = t_closure;
   p->lineno = lineno;
   p->rc = 1;
   p->opr.oper = LAMBDA;
   p->opr.nops = where == NULL ? 2 : 3;
   p->opr.env = envnew(e);
   p->opr.op[0] = params;
   p->opr.op[1] = body;
   p->opr.op[2] = where;
   return p;
}

node *car(node *node)
{
   struct node *ret;

   if (node->opr.nops > 0)
   {
      ret = node->opr.op[0];
      incref(ret);
   }
   else
      ret = mkpair(LIST, 0);

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
         ret = mkpair(LIST, 0);
         break;
      case 2:
         ret = node->opr.op[1];
         incref(ret);
         break;
   }

   decref(node);
   return ret;
}

bool empty(node *list)
{
   return list->type == t_pair && list->opr.nops == 0;
}

int length(node *node)
{
   if (node->type != t_pair)
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

node *cons(node *atom, node *list)
{
   if (empty(list))
      return mkpair(LIST, 1, atom);
   else
      return mkpair(LIST, 2, atom, list);
}

node *append(node *list1, node *list2)
{
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
   node *list = mkpair(LIST, 0);

   for (int i = to; i >= from; --i)
      list = cons(mkint(i), list);

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
		
      case t_pair:
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
      {
         printf("%s", symname(node->ival));
         break;
      }

      case t_tuple:
      {
         printf("{");
         
         for (int i = 0; i < node->tuple.count; ++i)
         {
            pprint(node->tuple.n[i]);
            doif(i < node->tuple.count - 1, printf(","));
         }
         
         printf("}");
         break;
      }
      
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
	
   incref(l1); //Compensate for the unwanted decref eq is
   incref(l2); //about to perform. Not 100% sure about this one.

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
   else if (x->type == t_pair)
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
