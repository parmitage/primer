#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "eval.h"
#include "y.tab.h"

int main(int argc, char **argv)
{
   if (argc != 2)
   {
      printf("usage: primer prog.pri\n");
      return -1;
   }

   init();
   eval(parse(argv[1]), top);

   return 0;
}

void init()
{
   /* initial library 'cache' has no items in it */
   lastlib = 0;

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
            return eval(b->node, e);
         else
            error("unbound symbol '%s'", symname(n->ival));
      }

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
         node *exp = eval(n->ast->n1, e);
         node *iter = n->ast->n2;

         while (iter != NULL)
         {
            node *match = CAR(iter);
            node *pattern = eval(CAR(match), e);

            if (is_any_pattern(pattern))
               return eval(CDR(match), e);

            node *equal = eq(exp, pattern);

            if (equal->ival == true)
               return eval(CDR(match), e);

            iter = CDR(iter);
         }

         error("non-exhaustive pattern");
      }

      case t_lambda:
      {
         return mkclosure(n->ast->n1, n->ast->n2, e);
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
   return e;
}

env *envdel(env *e)
{
   env *parent = e->parent;
   return parent;
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

struct node *prialloc()
{
   node *p;

   if ((p = (struct node*)malloc(sizeof(struct node))) == NULL)
      error("unable to allocate memory");

   return p;
}

node *mkint(int value)
{
   node *p = prialloc();
   p->type = t_int;
   p->ival = value;
   return p;
}

node *mkfloat(float value)
{
   node *p = prialloc();
   p->type = t_float;
   p->fval = value;
   return p;
}

node *mkbool(int value)
{
   node *p = prialloc();
   p->type = t_bool;
   p->ival = value;
   return p;
}

node* mkchar(char c)
{
   node *p = prialloc();
   p->type = t_char;
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

   str[i] = '\0';

   return str;
}

node *mksym(char* s)
{
   node *p = prialloc();
   p->type = t_symbol;
   p->ival = intern(s);
   return p;
}

node *mkpair(t_type type, node *car, node* cdr)
{
   node *p = prialloc();
   p->type = t_pair;
   p->pair = (struct pair*) malloc(sizeof(struct pair));
   p->pair->type = type;
   p->pair->car = car;
   p->pair->cdr = cdr;
   return p;
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

node *mkoperator(struct node * (*op) (struct node *), node *arg1)
{
   node *p = prialloc();
   p->type = t_operator;
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
   p->ast = (struct ast*)malloc(sizeof(struct ast));
   p->ast->n1 = n1;
   p->ast->n2 = n2;
   p->ast->n3 = n3;
   return p;
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

void using(node *args)
{
   ASSERT(args->type, t_symbol, "using requires a symbolic parameter");
   char *name = symname(args->ival);
   eval(loadlib(name), top);
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
   if (x->type != y->type)
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

void error(char* fmt, ...)
{
   // TODO wrapped format string can overflow its buffer
   char fmt2[1000];
   sprintf(fmt2, "error: %s\n", fmt);

   va_list args;
   va_start(args, fmt);
   vprintf(fmt2, args);
   va_end(args);
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

bool fexists(const char *path)
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
