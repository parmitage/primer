#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "utils.h"
#include "eval.h"
#include "y.tab.h"

/*
  TODO can pair use the type in node rather than in the pair struct (t_string becomes a type)?
  TODO simplification of some pair 'length' checking code
  TODO could evlis be simplified?
  TODO separate init code out of main
  TODO separate out ast nodes from runtime data structures (closures, lists, etc)?
  TODO hand written parser
  TODO REPL
  TODO reference counter
  TODO tail recursion check
  TODO build closure environment
 */

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

   top = envnew(NULL);

   /* character constants */
   extend(top, bindnew(intern("newline"), mkchar('\n')));
   extend(top, bindnew(intern("tab"), mkchar('\t')));

   /* type names */
   extend(top, bindnew(intern("int"), mkint(t_int)));
   extend(top, bindnew(intern("float"), mkint(t_float)));
   extend(top, bindnew(intern("bool"), mkint(t_bool)));
   extend(top, bindnew(intern("char"), mkint(t_char)));
   extend(top, bindnew(intern("list"), mkint(t_pair)));
   extend(top, bindnew(intern("string"), mkint(t_string)));
   extend(top, bindnew(intern("lambda"), mkint(t_closure)));

   lineno = 1;

   if (arg_loadlib == true)
      eval(loadlib(arg_stdlib), top);

   eval(parse(arg_fname), top);
   //envdel(top);

   trace("[inc=%ld, dec=%ld, alloc=%ld, free=%ld]",
         cnt_inc, cnt_dec, cnt_alloc, cnt_free);

   return 0;
}

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

      case t_def:
      {
         extend(e, bindnew(n->ast->n1->ival, eval(n->ast->n2, e)));
         break;
      }

      case t_lambda:
      {
         //env *ce = envnew(top);
         //build_closure_env(n->pair->cdr, env, ce);
         return mkclosure(n->ast->n1, n->ast->n2, n->ast->n3, e);
      }

      case t_operator:
      {
         if (n->op->arity == 2)
            return n->op->binop(eval(n->op->arg1, e), eval(n->op->arg2, e));
         else
            return n->op->op(eval(n->op->arg1, e));
      }

      case t_cons:
      {
         return cons(eval(n->ast->n1, e), eval(n->ast->n2, e));
      }

      case t_car:
      {
         return car(eval(n->ast->n1, e));
      }

      case t_cdr:
      {
         return cdr(eval(n->ast->n1, e));
      }

      case t_apply:
      {
         symbol fsym = n->ast->n1->ival;
         node *fn = eval(n->ast->n1, e);

         if (fn->type != t_closure)
            error("attempt to apply non function type");

         env *ext = envnew(fn->fn->env);

         /* parameters */
         node *args = n->ast->n2;
         bind(fn->fn->args, args, ext, e);

         /* where clause */
         if (fn->fn->where != NULL)
            eval(fn->fn->where, ext);

         /* function body */
         if (istailrecur(fn->fn->body, fsym))
         {
            if (tco_env != NULL)
               envdel(tco_env);
               
            n = fn->fn->body;
            e = tco_env = ext;
            goto eval_start;
         }
         else
         {
            node *ret = eval(fn->fn->body, ext);
            e = envdel(ext);
            return ret;
         }
      }

      case t_cond:
      {              			
         node *pred = eval(n->ast->n1, e);
         ASSERT(pred->type, t_bool, "type of predicate is not boolean");
					
         if (pred->ival > 0)
            n = n->ast->n2;
         else
            n = n->ast->n3;

         goto eval_start;
      }

      case t_seq:
      {
         eval(n->ast->n1, e);
         
         if (n->ast->n2 != NULL)
         {
            n = n->ast->n2;
            goto eval_start;
         }

         break;
      }

      case t_pair:
      {
         /* because we can store symbols in lists we evaluate the contents */
         node *ret = evlis(n, e);
         decref(n);
         incref(ret);
         return ret;
      }
   }
}

node *evlis(node *list, env *env)
{
   /* TODO this isn't quite right... */
   if (!CAR(list))
      return mkpair(list->pair->type, NULL, NULL);
   else if (!CDR(list))
      return mkpair(list->pair->type, eval(CAR(list), env), NULL);
   else
      return mkpair(list->pair->type, eval(CAR(list), env), evlis(CDR(list), env));
}

void bind(node *args, node *params, env *fnenv, env *argenv)
{
   if (params != NULL)
   {		
      if (CDR(params))
         bind(cdr(args), cdr(params), fnenv, argenv);
		
      if (CAR(params))
         extend(fnenv, bindnew(car(args)->ival, eval(car(params), argenv)));
   }
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
   SKIP_REF_COUNT;

   n->rc++;

   /* DEBUG */
   cnt_inc++;

   //printf("incref to %i for ", n->rc);
   //show(n);
}

void decref(node *n)
{
   SKIP_REF_COUNT;

   cnt_dec++;

   n->rc--;

   pprint(n);
   printf("rc=%i\n", n->rc);

   if (n->rc == 0)
   {
      free(n);
      cnt_free++;
   }

      /* while (n != NULL && n->pair != NULL && n->pair->car != NULL) */
      /* { */
      /*    n->pair->car->rc--; */

      /*    if (n->pair->car->rc == 0) */
      /*    { */
      /*       free(n->pair->car); */
      /*       cnt_free++; */
      /*    } */

      /*    n = n->pair->cdr; */
      /* } */
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

void extend(env *e, binding *nb)
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
            /*    extend(top, b); */
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
   switch (expr->type)
   {
      case t_int:
      case t_float:
      case t_bool:
      case t_char:
      case t_symbol:
         return true;
      case t_apply:
         return expr->ast->n1->ival == s;
      case t_cond:
         return istailrecur(expr->ast->n2, s)
            && istailrecur(expr->ast->n3, s);
      default:
         return false;
   }
}

struct node *prialloc()
{
   cnt_alloc++;
   node *p;

   if ((p = (struct node*)malloc(sizeof(struct node))) == NULL)
      error("Unable to allocate memory!");

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

node *mkint_literal(int value)
{
   node *p = mkint(value);
   p->rc = -1;
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
   return str_to_node(temp);
}

node* str_to_node(char* value)
{
   int len = strlen(value);

   if (len > 1)
      return mkpair(t_string, mkchar(value[0]), str_to_node(value + 1));
   else
      return mkpair(t_string, mkchar(value[0]), NULL);
}

char *node_to_str(node *node)
{
   char *str = (char*)malloc(50);
   int i = 0;

   while (node != NULL)
   {
      if (CAR(node))
      {
         str[i] = node->pair->car->ival;
         node = node->pair->cdr;
         ++i;
      }
      else
         node = NULL;
   }

   str[i] = '\n';
   return str;
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

node *mkpair(t_type type, node *car, node* cdr)
{
   node *p = prialloc();
   p->type = t_pair;
   p->lineno = lineno;
   p->rc = 1;
   p->pair = (struct pair*) malloc(sizeof(struct pair));
   p->pair->type = type;
   p->pair->car = car;
   p->pair->cdr = cdr;	
   return p;
}

node *mkclosure(node *args, node *body, node *where, env *env)
{
   node *p = prialloc();
   p->type = t_closure;
   p->lineno = lineno;
   p->rc = 1;
   p->fn = (struct closure*)malloc(sizeof(struct closure));
   p->fn->args = args;
   p->fn->body = body;
   p->fn->where = where;
   p->fn->env = envnew(env);
   return p;
}

node *mkoperator(struct node * (*op) (struct node *), node *arg1)
{
   node *p = prialloc();
   p->type = t_operator;
   p->lineno = lineno;
   p->rc = -1;
   p->op = (struct operator*)malloc(sizeof(struct operator));
   p->op->op = op;
   p->op->arity = 1;
   p->op->arg1 = arg1;
   p->op->arg2 = NULL;
   return p;
}

node *mkbinoperator(struct node * (*binop) (struct node *, struct node *), node *arg1, node *arg2)
{
   node *p = prialloc();
   p->type = t_operator;
   p->lineno = lineno;
   p->rc = -1;
   p->op = (struct operator*)malloc(sizeof(struct operator));
   p->op->binop = binop;
   p->op->arity = 2;
   p->op->arg1 = arg1;
   p->op->arg2 = arg2;
   return p;
}

node *mkast(t_type type, node *n1, node *n2, node *n3)
{
   node *p = prialloc();
   p->type = type;
   p->lineno = lineno;
   p->rc = -1;
   p->ast = (struct ast*)malloc(sizeof(struct ast));
   p->ast->n1 = n1;
   p->ast->n2 = n2;
   p->ast->n3 = n3;
   return p;
}

node *car(node *node)
{
   ASSERT(node->type, t_pair, "head can only be applied to lists");

   struct node *ret;

   if (CAR(node))
   {
      ret = CAR(node);
      incref(ret);
   }
   else
      ret = mkpair(t_pair, NULL, NULL);

   //decref(node);
   return ret;
}

node *cdr(node *node)
{
   ASSERT(node->type, t_pair, "tail can only be applied to lists");

   /* TODO could this be simplified? */
   struct node *ret;

   if (CDR(node))
      ret = CDR(node);
   else
      ret = mkpair(t_pair, NULL, NULL);

   return ret;
}

node *len(node *node)
{
   ASSERT(node->type, t_pair, "length can only be applied to lists");

   int n = 0;
   struct node *iter = node;

   while (iter != NULL && CAR(iter))
   {
      n += 1;
      iter = iter->pair->cdr;
   }

   decref(node);

   return mkint(n);
}

node *at(node *arg1, node *arg2)
{
   node *list = arg1;
   int index = arg2->ival;

   ASSERT(arg1->type, t_pair, "left operand to at must be a list");
   ASSERT(arg2->type, t_int, "right operand to at must be an integer");

   int n = 0;
   bool found = false;

   while (!found)
   {
      if (list == NULL || index < 0)
      {
         return mkpair(t_pair, NULL, NULL);
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
}

node *cons(node *atom, node *list)
{
   ASSERT(list->type, t_pair, "right operand to cons must be a list");

   if (EMPTY(list))
      return mkpair(t_pair, atom, NULL);
   else
      return mkpair(t_pair, atom, list);
}

node *append(node *list1, node *list2)
{
   ASSERT(list1->type, t_pair, "left operand to append must be a list");
   ASSERT(list2->type, t_pair, "right operand to append must be a list");

   if (EMPTY(list2))
      return list1;

   if (EMPTY(list1))
      return list2;

   node *r = list1;
   node *n = list1;
	
   while(n->pair->cdr != NULL && !EMPTY(n->pair->cdr))
   {
      n = n->pair->cdr;
   }
	
   n->pair->cdr = list2;
   
   return r;
}

node *range(node *s, node *e)
{
   ASSERT(s->type, t_int, "left operand to range must be an integer");
   ASSERT(e->type, t_int, "right operand to range must be an integer");

   int from = s->ival;
   int to = e->ival;
   node *list = mkpair(t_pair, NULL, NULL);

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
      case t_symbol:
         printf("%s", symname(node->ival));
         break;
      case LAMBDA:
         printf("#<lambda>");
         break;
      case t_closure:
         printf("#<closure>");
         break;
      case t_pair:
         switch (node->pair->type)
         {
            case t_string:
               printf("\"");

               while (node != NULL)
               {
                  if (CAR(node))
                  {
                     printf("%c", node->pair->car->ival);
                     node = node->pair->cdr;
                  }
                  else
                     node = NULL;
               }
           
               printf("\"");
               break;
		
            case t_pair:
               printf("[");
           
               while (node != NULL)
               {
                  if (CAR(node))
                  {
                     pprint(node->pair->car);
                     if (CDR(node))
                        printf(",");
                     node = node->pair->cdr;
                  }
                  else
                     node = NULL;
               }
					
               printf("]");
               break;
         }
         break;
   }
}

node *show(node *args)
{
   pprint(args);
   printf("\n");
   return args;
}

node *add(node *x, node *y)
{
   ASSERT_NUM(x, "left operand to + must be numeric");
   ASSERT_NUM(y, "right operand to + must be numeric");

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
   ASSERT_NUM(x, "left operand to - must be numeric");
   ASSERT_NUM(y, "right operand to - must be numeric");

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

node *neg(node *x)
{
   ASSERT_NUM(x, "operand to - must be numeric");

   node *ret;

   switch (x->type)
   {
      case t_int:
         ret = mkint(-x->ival);
         break;
      case t_float:
         ret = mkfloat(-x->fval);
         break;
   }

   decref(x);

   return ret;
}

node *mul(node *x, node *y)
{
   ASSERT_NUM(x, "left operand to * must be numeric");
   ASSERT_NUM(y, "right operand to * must be numeric");

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
   ASSERT_NUM(x, "left operand to / must be numeric");
   ASSERT_NUM(y, "right operand to / must be numeric");

   node *ret;

   switch (DIVIDE_RETURN_TYPE(x, y))
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
   node *ret = EXTRACT_NUMBER(x) > EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;

   decref(x);
   decref(y);

   return ret;
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
   node *ret = EXTRACT_NUMBER(x) >= EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
   
   decref(x);
   decref(y);
   return ret;
}

node *list_eq(node *l1, node *l2)
{
   if (EMPTY(l1) && EMPTY(l2))
      return NODE_BOOL_TRUE;

   /* TODO can this be simplified? */
   if ((CAR(l1) && EMPTY(l2)) ||
       (EMPTY(l1) && CAR(l2)) ||
       (CDR(l1) && !CDR(l2)) ||
       (!CDR(l1) && CDR(l2)))
      return NODE_BOOL_FALSE;
	
   incref(l1); //Compensate for the unwanted decref eq is
   incref(l2); //about to perform. Not 100% sure about this one.

   if (eq(l1->pair->car, l2->pair->car)->ival == true)
   {
      if (CDR(l1) && CDR(l2))
         return list_eq(l1->pair->cdr, l2->pair->cdr);
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
   return eq(x, y)->ival == true ? NODE_BOOL_FALSE : NODE_BOOL_TRUE;
}

node *and(node *x, node *y)
{
   ASSERT(x->type, t_bool, "left operand to and must be boolean");
   ASSERT(y->type, t_bool, "right operand to and must be boolean");

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
   ASSERT(x->type, t_bool, "left operand to or must be boolean");
   ASSERT(y->type, t_bool, "right operand to or must be boolean");

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
   ASSERT(x->type, t_bool, "operand to not must be boolean");

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
   ASSERT(x->type, t_int, "left operand to mod must be integer");
   ASSERT(y->type, t_int, "right operand to mod must be integer");

   node *retval = mkint(x->ival % y->ival);

   decref(x);
   decref(y);
   return retval;
}

node *is(node *exp, node *type)
{
   return mkbool(exp->type == type->ival);
}

node *as(node *from, node *to)
{
   int target = to->ival;
   char buffer[10];
   int ival;
   float fval;

   if (from->type == target)
      return from;

   switch (from->type)
   {
      case t_int:
      {
         switch (target)
         {
            case t_string:
            {
               ival = from->ival;
               sprintf(buffer, "%d", ival);
               return str_to_node(buffer);
            }
            
            case t_float:
            {
               fval = from->ival;
               return mkfloat(fval);
            }

            case t_bool:
            {
               if (from->ival <= 0)
                  return NODE_BOOL_FALSE;
               else
                  return NODE_BOOL_TRUE;
            }

            default:
               error("conversion from integer to unsupported type");
         }
      }

      case t_float:
      {
         switch (target)
         {
            case t_string:
            {
               fval = from->fval;
               sprintf(buffer, "%g", fval);
               return str_to_node(buffer);
            }

            case t_int:
            {
               return mkint((int)from->fval);
            }

            default:
               error("conversion from float to unsupported type");
         }
      }

      case t_bool:
      {
         switch (target)
         {
            case t_int:
               return mkint(from->ival);

            case t_float:
               return mkfloat(from->ival);

            case t_string:
               return str_to_node(from->ival > 0 ? "true" : "false");

            default:
               error("cast from bool to unsupported type");
         }
      }

      case t_char:
      {
         switch (target)
         {
            case t_string:
            {
               ival = from->ival;
               sprintf(buffer, "%c", ival);
               return str_to_node(buffer);
            }

            case t_int:
            {
               ival = from->ival;
               return mkint(ival);
            }

            case t_float:
            {
               fval = from->ival;
               return mkfloat(fval);
            }

            default:
               error("attempted cast from char to unsupported type");
         }
      }

      case t_pair:
      {
         if (from->pair->type == t_string)
         {
            char *str = node_to_str(from);

            switch (target)
            {
               case t_int:
               {
                  ival = atoi(str);
                  return mkint(ival);
               }

               case t_float:
               {
                  fval = atof(str);
                  return mkfloat(fval);
               }

               case t_char:
               {
                  ival = atoi(str);
                  return mkchar(str[0]);
               }

               default:
                  error("attempted cast from string to unsupported type");
            }
         }
         else
            error("lists can not be cast to other types");
      }

      default:
         error("cast from unsupported type");
   }
}

node *loadlib(char *name)
{
   char *libroot, libpath[500];
   libroot = getenv("PRIMER_LIBRARY_PATH");
  	
   if (libroot == NULL)
      error("The environment variable PRIMER_LIBRARY_PATH has not been set");
    
   sprintf(libpath, "%s%s.pri", libroot, name);
    
   if (!fexists(libpath))
      error("Unable to find standard library");
  
   return parse(libpath);
}

void error(char *msg)
{
   printf("error: %s\n", msg);
   exit(-1);
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
