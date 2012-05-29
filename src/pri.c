#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "main.h"
#include "pri.h"
#include "y.tab.h"
#include "gc.h"

int main(int argc, char **argv)
{
   if (argc != 2)
   {
      printf("usage: pri prog.pri\n");
      return -1;
   }

   init_stage1();
   node *n = parse(argv[1]);
   init_stage2();
   eval(n, top);

   return 0;
}

void init_stage1()
{
   /* initial library 'cache' has no items in it */
   lastlib = 0;

   /* Initialise the garbage collector but immediatly disable collection
      as we don't want anything to be collected during read time (if a program
      text is larger than the available heap then the memory allocated will
      most likely be collected because none of the data is interned during
      read time so it is unreachable by the mark phase of the collector).
      Disabling the collector means that the memory allocated comes directly
      from the C heap rather than Primer's managed heap and so will never 
      be collected. */
   GC_init();
   GC_disable();

   srand((unsigned)(time(0)));

   NODE_BOOL_TRUE = mkbool(true);
   NODE_BOOL_FALSE = mkbool(false);

   /* the magic '_' value used in pattern matching */
   NODE_ANY = mkbool(-1);

   /* top level environment has no parent */
   top = envnew(NULL);

   extend(top, bindnew(intern("newline"), mkchar('\n')));
   extend(top, bindnew(intern("tab"), mkchar('\t')));

   extend(top, bindnew(intern("int"), mkint(t_int)));
   extend(top, bindnew(intern("float"), mkint(t_float)));
   extend(top, bindnew(intern("bool"), mkint(t_bool)));
   extend(top, bindnew(intern("char"), mkint(t_char)));
   extend(top, bindnew(intern("list"), mkint(t_pair)));
   extend(top, bindnew(intern("string"), mkint(t_string)));
   extend(top, bindnew(intern("lambda"), mkint(t_closure)));
}

void init_stage2()
{
   //GC_enable();
}

node *eval(node *n, env *e)
{
   if (!n)
      return NODE_BOOL_FALSE;

  eval_start:

   /* It's important that we track the 'current environment' so that the
      garbage collector knows where it should walk roots from. However,
      I'm not convinced that this is the correct or only place that this
      needs to be set. */
   cenv = e;

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
            return eval(b->node, e);
         else
            error("unbound symbol '%s'", symname(n->ival));
      }

      /**********************************************************************
                                special operators
      **********************************************************************/

      case t_val:
      {
         extend(e, bindnew(n->ast->n1->ival, eval(n->ast->n2, e)));
         break;
      }

      case t_let:
      {
         env *ext = envnew(e);
         node *bindings = n->ast->n1;

         while (bindings != NULL)
         {
            node *exp = bindings->ast->n1;
            extend(ext, bindnew(exp->ast->n1->ival, eval(exp->ast->n2, ext)));
            bindings = bindings->ast->n2;
         }

         return eval(n->ast->n2, ext);
      }

      case t_match:
      {
         /* Some terminology:

            match e1, e2
               with p1a, p1b then r1
               with p2a, p2b then r2
               with _, _     then r3

            e1                    => a test
            e1, e2                => the tests
            with p1a, p1b then r1 => a case
            with..with..with      => the cases
            p1a                   => a pattern   
            p1a, p1b              => the patterns
            r1                    => a result
          */

         node *lhs = n->ast->n1;
         node *tests = lhs;
         node *cases = n->ast->n2;

         while (cases != NULL)
         {
            node *acase = CAR(cases);
            node *patterns = CAR(acase);
            bool pass = true;

            while (tests != NULL && patterns != NULL)
            {
               node *test = CAR(tests);
               node *pattern = CAR(patterns);

               node *n1 = eval(test, e);
               node *n2 = eval(pattern, e);
               pass = eq(n1, n2)->ival;

               if (!pass)
                  break;               

               tests = CDR(tests);
               patterns = CDR(patterns);
            }

            if (pass)
               return eval(CDR(acase), e);

            cases = CDR(cases);
            tests = lhs;
         }

         error("non-exhaustive patterns in match statement");
      }

      case t_lambda:
      {
         return mkclosure(n->ast->n1, n->ast->n2, e);
      }

      case t_using:
      {
         using(n->ast->n1);
         break;
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
            GC_recursive_markbit_set(ret, 0);
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
         node *ret = evlis(n, e);
         return ret;
      }

      /**********************************************************************
                                binary operators
      **********************************************************************/

      case t_add:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return add(lhs, rhs);
      }

      case t_sub:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return sub(lhs, rhs);
      }

      case t_mul:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return mul(lhs, rhs);
      }

      case t_dvd:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return dvd(lhs, rhs);
      }

      case t_lt:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return lt(lhs, rhs);
      }

      case t_gt:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return gt(lhs, rhs);
      }

      case t_gte:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return gte(lhs, rhs);
      }

      case t_lte:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return lte(lhs, rhs);
      }

      case t_neq:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return neq(lhs, rhs);
      }

      case t_eq:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return eq(lhs, rhs);
      }

      case t_and:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return and(lhs, rhs);
      }

      case t_or:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return or(lhs, rhs);
      }

      case t_b_and:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return b_and(lhs, rhs);
      }

      case t_b_or:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return b_or(lhs, rhs);
      }

      case t_b_xor:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return b_xor(lhs, rhs);
      }

      case t_b_lshift:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return b_lshift(lhs, rhs);
      }

      case t_b_rshift:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return b_rshift(lhs, rhs);
      }

      case t_mod:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return mod(lhs, rhs);
      }

      case t_append:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return append(lhs, rhs);
      }

      case t_range:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return range(lhs, rhs);
      }

      case t_at:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return at(lhs, rhs);
      }

      case t_as:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return as(lhs, rhs);
      }

      case t_is:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return is(lhs, rhs);
      }

      case t_cons:
      {
         node *lhs = eval(n->ast->n1, e);
         node *rhs = eval(n->ast->n2, e);
         return cons(lhs, rhs);
      }

      /**********************************************************************
                                unary operators
      **********************************************************************/

      case t_car:
      {
         return car(eval(n->ast->n1, e));
      }

      case t_cdr:
      {
         return cdr(eval(n->ast->n1, e));
      }

      case t_neg:
      {
         return neg(eval(n->ast->n1, e));
      }

      case t_show:
      {
         node *ret = eval(n->ast->n1, e);
         show(ret);
         return ret;
      }

      case t_reads:
      {
         return reads();
      }

      case t_len:
      {
         return len(eval(n->ast->n1, e));
      }

      case t_rnd:
      {
         return rnd(eval(n->ast->n1, e));
      }

      case t_not:
      {
         return not(eval(n->ast->n1, e));
      }

      case t_bnot:
      {
         return b_not(eval(n->ast->n1, e));
      }
   }
}

node *evlis(node *list, env *env)
{
   /* This is basically McCarthy's original evlis function with a small but
      significant optimisation to avoid evaluating if unecessary: if a list
      contains symbols or unevaluated functions then it must be evaluated
      before use. If we can avoid evaluating the list contents then we can
      reduce consing by several orders of magnitude in list heavy programs. */

   node *iter = list;
   bool primitive = true;

   while (iter != NULL)
   {
      if (CAR(iter))
      {
         node *n = CAR(iter);

         if (n->type == t_int || n->type == t_float ||
             n->type == t_char || n->type == t_bool ||
             (n->type == t_pair && n->pair->type == t_string))
         {
            iter = iter->pair->cdr;
         }
         else
         {
            iter = NULL;
            primitive = false;
         }
            
      }
      else
         iter = NULL;
   }

   if (primitive)
   {
      return list;
   }
   else
   {
      if (!CAR(list))
         return mkpair(list->pair->type, NULL, NULL);
      else if (!CDR(list))
         return mkpair(list->pair->type, eval(CAR(list), env), NULL);
      else
         return mkpair(list->pair->type, eval(CAR(list), env), evlis(CDR(list), env));
   }
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
   cenv = e;
   return e;
}

env *envdel(env *e)
{
   env *parent = e->parent;
   cenv = parent;
   return parent;
}

void envprint(env *e, bool depth)
{
   while (e != NULL)
   {
      binding *b = e->bind;

      while (b != NULL)
      {
         printf("%s <-", symname(b->sym));
         pprint(b->node);
         printf("\n");

         b = b->prev;
      }

      printf("-----------------\n");

      if (depth)
         e = e->parent;
      else
         break;
   }
}

void extend(env *e, binding *nb)
{
   symbol s = nb->sym;
   binding *h = e->bind;
   
   while (h != NULL)
   {
      if (s == h->sym)
         error("symbol '%s' already bound in this environment", symname(s));

      h = h->prev;
   }

   /* binding wasn't found so create a new one */
   nb->prev = e->bind;
   e->bind = nb;

   /* object has been fully constructed and bound so is
      eligible for inspection by the gc */
   GC_recursive_markbit_set(nb->node, 0);
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
            return b;

         b = b->prev;
      }

      e = e->parent;
      depth = true;
   }
  
   return NULL;
}

node *mkclosure(node *args, node *body, env *env)
{
   node *p = prialloc();
   p->type = t_closure;
   p->fn = (struct closure*)malloc(sizeof(struct closure));
   p->fn->args = args;
   p->fn->body = body;
   p->fn->env = envnew(env);
   return p;
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
      case t_pair:
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

node *car(node *node)
{
   ASSERT(node->type, t_pair, "head can only be applied to lists");

   if (CAR(node))
      return CAR(node);
   else
      return mkpair(t_pair, NULL, NULL);
}

node *cdr(node *node)
{
   ASSERT(node->type, t_pair, "tail can only be applied to lists");

   if (CDR(node))
      return CDR(node);
   else
      return mkpair(t_pair, NULL, NULL);
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

   return mkint(n);
}

node *rnd(node *node)
{
   ASSERT(node->type, t_int, "rnd requires an integer parameter");
   int limit = node->ival;
   return mkint(rand() % node->ival);
}

node *at(node *arg1, node *arg2)
{
   node *list = arg1;
   int index = arg2->ival;

   ASSERT(arg1->type, t_pair, "left operand to at must be a list");
   ASSERT(arg2->type, t_int, "right operand to at must be an integer");

   int n = 0;
   bool found = false;
   node *ret;

   while (!found)
   {
      if (list == NULL || index < 0)
      {
         ret = mkpair(t_pair, NULL, NULL);
         found = true;
      }
      else if (index == n)
      {
         ret = car(list);
         found = true;
      }
      else
      {
         list = cdr(list);
         ++n;
      }
   }

   return ret;
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

   node *ptr = list1;
   node *copy = mkpair(t_pair, CAR(ptr), NULL);
   node *head = copy;
	
   while (CDR(ptr) != NULL && !EMPTY(CDR(ptr)))
   {
      copy->pair->cdr = mkpair(t_pair, CADR(ptr), NULL);
      copy = CDR(copy);
      ptr = CDR(ptr);
   }

   ptr = list2;
   copy->pair->cdr = mkpair(t_pair, CAR(ptr), NULL);
   copy = CDR(copy);

   while (CDR(ptr) != NULL && !EMPTY(CDR(ptr)))
   {
      copy->pair->cdr = mkpair(t_pair, CADR(ptr), NULL);
      copy = CDR(copy);
      ptr = CDR(ptr);
   }
	
   return head;
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

node *reads()
{
   char input[81];

   if (NULL != fgets(input, sizeof input, stdin))
   {
      char *nlptr = strchr(input, '\n');
      if (nlptr) *nlptr = '\0';
   }

   return str_to_node(input);
}

void using(node *args)
{
   ASSERT(args->type, t_symbol, "using requires a symbolic parameter");
   char *name = symname(args->ival);
   GC_disable();
   eval(loadlib(name), top);
   //GC_enable();
}

node *add(node *x, node *y)
{
   ASSERT_NUM(x, "left operand to + must be numeric");
   ASSERT_NUM(y, "right operand to + must be numeric");

   switch (NUMERIC_RETURN_TYPE(x, y))
   {
      case t_int:
         return mkint(EXTRACT_NUMBER(x) + EXTRACT_NUMBER(y));
      case t_float:
         return mkfloat(EXTRACT_NUMBER(x) + EXTRACT_NUMBER(y));
   }
}

node *sub(node *x, node *y)
{
   ASSERT_NUM(x, "left operand to - must be numeric");
   ASSERT_NUM(y, "right operand to - must be numeric");

   switch (NUMERIC_RETURN_TYPE(x, y))
   {
      case t_int:
         return mkint(EXTRACT_NUMBER(x) - EXTRACT_NUMBER(y));
      case t_float:
         return mkfloat(EXTRACT_NUMBER(x) - EXTRACT_NUMBER(y));
   }
}

node *neg(node *x)
{
   ASSERT_NUM(x, "operand to - must be numeric");

   switch (x->type)
   {
      case t_int:
         return mkint(-x->ival);
      case t_float:
         return mkfloat(-x->fval);
   }
}

node *mul(node *x, node *y)
{
   ASSERT_NUM(x, "left operand to * must be numeric");
   ASSERT_NUM(y, "right operand to * must be numeric");

   switch (NUMERIC_RETURN_TYPE(x, y))
   {
      case t_int:
         return mkint(EXTRACT_NUMBER(x) * EXTRACT_NUMBER(y));
      case t_float:
         return mkfloat(EXTRACT_NUMBER(x) * EXTRACT_NUMBER(y));
   }
}

node *dvd(node *x, node *y)
{
   ASSERT_NUM(x, "left operand to / must be numeric");
   ASSERT_NUM(y, "right operand to / must be numeric");

   switch (DIVIDE_RETURN_TYPE(x, y))
   {
      case t_int:
         return mkint(EXTRACT_NUMBER(x) / EXTRACT_NUMBER(y));
      case t_float:
         return mkfloat(EXTRACT_NUMBER(x) / EXTRACT_NUMBER(y));
   }
}

node *lt(node *x, node *y)
{
   return EXTRACT_NUMBER(x) < EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
}

node *gt(node *x, node *y)
{
   return EXTRACT_NUMBER(x) > EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
}

node *lte(node *x, node *y)
{
   return EXTRACT_NUMBER(x) <= EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
}

node *gte(node *x, node *y)
{
   return EXTRACT_NUMBER(x) >= EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
}

node *list_eq(node *l1, node *l2)
{
   if (EMPTY(l1) && EMPTY(l2))
      return NODE_BOOL_TRUE;

   if ((CAR(l1) && EMPTY(l2)) || (EMPTY(l1) && CAR(l2)) ||
       (CDR(l1) && !CDR(l2)) || (!CDR(l1) && CDR(l2)))
      return NODE_BOOL_FALSE;
	
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
   if (ASSERT_ANY(x) || ASSERT_ANY(y))
      return NODE_BOOL_TRUE;
   else if (x->type != y->type)
      return NODE_BOOL_FALSE;
   else if (x->type == t_int || x->type == t_bool)
      return x->ival == y->ival ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
   else if (x->type == t_float)
      return x->fval == y->fval ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
   else if (x->type == t_char)
      return x->ival == y->ival ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
   else if (x->type == t_pair)
      return list_eq(x, y);
   else
      return NODE_BOOL_FALSE;
}

node *neq(node *x, node *y)
{
   return eq(x, y)->ival == true ? NODE_BOOL_FALSE : NODE_BOOL_TRUE;
}

node *and(node *x, node *y)
{
   ASSERT(x->type, t_bool, "left operand to and must be boolean");
   ASSERT(y->type, t_bool, "right operand to and must be boolean");

   if (x == NODE_BOOL_FALSE || y == NODE_BOOL_FALSE)
      return NODE_BOOL_FALSE;
   else
      return NODE_BOOL_TRUE;
}

node *or(node *x, node *y)
{
   ASSERT(x->type, t_bool, "left operand to or must be boolean");
   ASSERT(y->type, t_bool, "right operand to or must be boolean");

   if (x->ival || y->ival)
      return NODE_BOOL_TRUE;
   else
      return NODE_BOOL_FALSE;
}

node *not(node *x)
{
   ASSERT(x->type, t_bool, "operand to not must be boolean");

   if (x->ival == true)
      return NODE_BOOL_FALSE;
   else
      return NODE_BOOL_TRUE;
}

node *mod(node *x, node *y)
{
   ASSERT(x->type, t_int, "left operand to mod must be integer");
   ASSERT(y->type, t_int, "right operand to mod must be integer");
   return mkint(x->ival % y->ival);
}

node *b_or(node *x, node *y)
{
   return mkint(x->ival | y->ival);
}

node *b_and(node *x, node *y)
{
   return mkint(x->ival & y->ival);
}

node *b_xor(node *x, node *y)
{
   return mkint(x->ival | y->ival);
}

node *b_not(node *x)
{
   return mkint(~x->ival);
}

node *b_lshift(node *x, node *y)
{
   return mkint(x->ival << y->ival);
}

node *b_rshift(node *x, node *y)
{
   return mkint(x->ival >> y->ival);
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
               break;
         }
         break;
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
               break;
         }
         break;
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
               break;
         }
         break;
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
               break;
         }
         break;
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
                  break;
            }
         }
         else
            error("lists can not be cast to other types");

         break;
      }

      default:
         error("cast from unsupported type");
         break;
   }
}

bool is_any_pattern(node *n)
{
   return n->type == t_bool && n->ival == -1;
}

bool cached(char *name)
{
   /* search the 'cache' to see if a library has already been loaded */

   int i;

   for (i = 0; i < lastlib; ++i)
   {
      if (strcmp(libcache[i], name) == 0)
         return true;
   }

   libcache[i] = (char*)malloc(sizeof(char) * (strlen(name) + 1));
   strcpy(libcache[i], name);
   lastlib++;

   return false;   
}

node *loadlib(char *name)
{
   if (cached(name))
      return NODE_BOOL_FALSE;  // TODO a unit type would be nice here

   char *libroot, libpath[500];
   libroot = getenv("PRIMER_LIBRARY_PATH");
  	
   if (libroot == NULL)
      error("The environment variable PRIMER_LIBRARY_PATH has not been set");
    
   sprintf(libpath, "%s%s.pri", libroot, name);
    
   if (!fexists(libpath))
      error("unable to find library '%s'", name);
  
   return parse(libpath);
}
