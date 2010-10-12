#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "utils.h"
#include "eval.h"
#include "y.tab.h"

/*
  TODO hand written parser
  TODO intern all builtins into top environment and change operators to not store args
  TODO proper CONS cells
  TODO REPL
  TODO reference counter
  TODO proper tail recursion checks
  TODO proper build closure environment
  TODO merge lambda and closure structs so that lambda->closure is simpler
  TODO tidy up bind, bindarg and bindp
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

   lineno = 1;
   wildcard = intern("_");

   if (arg_loadlib == true)
      eval(loadlib(arg_stdlib), top);

   eval(parse(arg_fname), top);
   envdel(top);

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

      case t_lambda:
      {
         //env *ce = envnew(top);
         //build_closure_env(n->opr.op[1], env, ce);
         return mkclosure(n->lambda->args, n->lambda->body, n->lambda->where, e);
      }

      case t_operator:
      {
         /* TODO this use of mkpair would be tidied up when we have proper cons cells */
         return n->op->primitive(mkpair(-1, 2, eval(n->op->arg1, e), eval(n->op->arg2, e)));
      }

      case t_apply:
      {
         symbol fsym = n->apply->fn->ival;
         node *fn = eval(n->apply->fn, e);
         env *ext = envnew(fn->fn->env);

         /* parameters */
         node *args = n->apply->args;
         bindarg(fn->fn->args, args, ext, e);

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
            //e = envdel(ext);
            return ret;
         }
      }

      case t_cond:
      {              			
         node *pred = eval(n->cond->predicate, e);
         ASSERT(pred->type, t_bool, "type of predicate is not boolean");
					
         if (pred->ival > 0)
            n = n->cond->consequent;
         else
            n = n->cond->alternate;

         goto eval_start;
      }

      case t_seq:
      {
         eval(n->seq->this, e);
         
         if (n->seq->next != NULL)
         {
            n = n->seq->next;
            goto eval_start;
         }

         break;
      }

      case t_pair:
      {
         switch(n->opr.oper)
         {
            /* TODO DEF is difficult to convert to a type because bind is expecting
               a cons to be passed as it's compaitble with parameter binding */
            case DEF:
            {
               bind(n, eval(n->opr.op[1], e), e);
               break;
            }

            case LIST:
            case STRING:
            {
               /* because we can store symbols in lists we evaluate the contents */
               node *ret;
               
               switch (n->opr.nops)
               {
                  case 0:
                     ret = mkpair(n->opr.oper, 0);
                     break;
                  case 1:
                     ret = mkpair(n->opr.oper, 1, eval(n->opr.op[0], e));
                     break;
                  case 2:
                     ret = mkpair(n->opr.oper, 2, eval(n->opr.op[0], e), eval(n->opr.op[1], e));
                     break;
               }

               decref(n);
               //incref(ret);
               return ret;
            }
           
            /* TODO CONS is difficult to convert to a type because CONS
               is used in the binding logic... */
            case CONS:
               return cons(eval(n->opr.op[0], e), eval(n->opr.op[1], e));
         }
      }
   }
}

void bindarg(node *args, node *params, env *fnenv, env *argenv)
{
   if (params != NULL)
   {		
      if (params->opr.nops > 1)
         bindarg(args->opr.op[1], params->opr.op[1], fnenv, argenv);
		
      if (params->opr.nops > 0)
         bind(args, eval(params->opr.op[0], argenv), fnenv);
   }
}

void bind(node *lhs, node *rhs, env *env)
{
    if (lhs->opr.op[0]->type == t_symbol)
    {
       binding *b = bindnew(lhs->opr.op[0]->ival, rhs);
       extend(env, b);
    }
    else if (lhs->opr.op[0]->type == t_pair && lhs->opr.op[0]->opr.oper == CONS)
       bindp(lhs->opr.op[0], rhs, env);
}

void bindp(node *args, node *list, env *fnenv)
{
   node *head = car(list);
   node *rest = cdr(list);

   if (args->opr.op[0]->ival != wildcard)
   {
      binding *headb = bindnew(args->opr.op[0]->ival, head);
      extend(fnenv, headb);
   }

   if (args->opr.op[1]->type == t_symbol)
   {
      if (args->opr.op[1]->ival != wildcard)
      {
         binding *restb = bindnew(args->opr.op[1]->ival, rest);
         extend(fnenv, restb);
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
   SKIP_REF_COUNT

   n->rc++;

   /* DEBUG */
   cnt_inc++;

   //printf("incref to %i for ", n->rc);
   //show(n);
}

void decref(node *n)
{
   SKIP_REF_COUNT

   n->rc--;

   /* DEBUG */
   cnt_dec++;

   //printf("decref to %i for ", n->rc);
   //show(n);

   if (n->rc == 0)
   {
      /* DEBUG */
      cnt_free++;

      if (n->type == t_pair && (n->opr.oper == LIST || n->opr.oper == STRING))
      {
         for (int i = 0; i < n->opr.nops; i++)
            decref(n->opr.op[i]);
      }

      //free(n);
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

node *mklambda(node *args, node *body, node *where)
{
   node *p = prialloc();
   p->type = t_lambda;
   p->lineno = lineno;
   p->rc = 1;
   p->lambda = (struct lambda*)malloc(sizeof(struct lambda));
   p->lambda->args = args;
   p->lambda->body = body;
   p->lambda->where = where;
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

node *mkoperator(struct node * (*op) (struct node *), node *arg1, node *arg2)
{
   node *p = prialloc();
   p->type = t_operator;
   p->lineno = lineno;
   p->rc = -1;
   p->op = (struct operator*)malloc(sizeof(struct operator));
   p->op->primitive = op;
   p->op->arg1 = arg1;
   p->op->arg2 = arg2;
   return p;
}

node *mkapply(node *fn, node *args)
{
   node *p = prialloc();
   p->type = t_apply;
   p->lineno = lineno;
   p->rc = -1;
   p->apply = (struct apply*)malloc(sizeof(struct apply));
   p->apply->fn = fn;
   p->apply->args = args;
   return p;
}

node *mkcond(node *predicate, node *consequent, node *alternate)
{
   node *p = prialloc();
   p->type = t_cond;
   p->lineno = lineno;
   p->rc = -1;
   p->cond = (struct cond*)malloc(sizeof(struct cond));
   p->cond->predicate = predicate;
   p->cond->consequent = consequent;
   p->cond->alternate = alternate;
   return p;
}

node *mkseq(node *this, node *next)
{
   node *p = prialloc();
   p->type = t_seq;
   p->lineno = lineno;
   p->rc = -1;
   p->seq = (struct seq*)malloc(sizeof(struct seq));
   p->seq->this = this;
   p->seq->next = next;
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

node *len(node *node)
{
   node = node->opr.op[0];
   ASSERT(node->type, t_pair, "argument to length must be a list");

   int n = 0;
   struct node *iter = node;

   while (iter != NULL && iter->opr.nops > 0)
   {
      n += 1;
      iter = iter->opr.op[1];
   }

   decref(node);

   return mkint(n);
}

node *nth(node *args)
{
   node *list = args->opr.op[0];
   int index = args->opr.op[1]->ival;

   int n = 0;
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
}

node *cons(node *atom, node *list)
{
   if (empty(list))
      return mkpair(LIST, 1, atom);
   else
      return mkpair(LIST, 2, atom, list);
}

node *append(node *args)
{
   node *list1 = args->opr.op[0];
   node *list2 = args->opr.op[1];

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

node *range(node *args)
{
   node *s = args->opr.op[0];
   node *e = args->opr.op[1];

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
      case t_symbol:
         printf("%s", symname(node->ival));
         break;
      case t_closure:
         printf("#<closure>");
         break;
      case t_pair:
         switch (node->opr.oper)
         {
            case STRING:
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
		
            case LIST:
               printf("[");
           
               while (node != NULL)
               {
                  if (node->opr.nops > 0)
                  {
                     pprint(node->opr.op[0]);
                     if (node->opr.nops > 1)
                        printf(",");
                     node = node->opr.op[1];
                  }
                  else
                     node = NULL;
               }
					
               printf("]");
               break;

            case LAMBDA:
               printf("#<lambda>");
               break;
         }
         break;
   }
}

node *show(node *args)
{
   pprint(args->opr.op[0]);
   printf("\n");
   return args->opr.op[0];
}

node *add(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];

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

   // decref(args)
   // decref(x);
   // decref(y);
   return ret;
}

node *sub(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];

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

node *neg(node *args)
{
   node *x = args->opr.op[0];

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

node *mul(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];
   
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

   //decref(x);
   //decref(y);
   return ret;
}

node *dvd(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];
   
   ASSERT_NUM(x, "left operand to / must be numeric");
   ASSERT_NUM(y, "right operand to / must be numeric");

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

node *lt(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];

   node *ret = EXTRACT_NUMBER(x) < EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;

   //decref(x);
   //decref(y);

   return ret;
}

node *gt(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];

   node *ret = EXTRACT_NUMBER(x) > EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;

   //decref(x);
   //decref(y);

   return ret;
}

node *lte(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];
   node *ret = EXTRACT_NUMBER(x) <= EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
   
   //decref(x);
   //decref(y);
   return ret;
}

node *gte(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];
   node *ret = EXTRACT_NUMBER(x) >= EXTRACT_NUMBER(y) ? NODE_BOOL_TRUE : NODE_BOOL_FALSE;
   
   //decref(x);
   //decref(y);
   return ret;
}

node *list_eq(node *l1, node *l2)
{
   if (l1->opr.nops == 0 && l2->opr.nops == 0)
      return NODE_BOOL_TRUE;

   if (l1->opr.nops != l2->opr.nops)
      return NODE_BOOL_FALSE;
	
   incref(l1); //Compensate for the unwanted decref eq is
   incref(l2); //about to perform. Not 100% sure about this one.

   if (eq(mkpair(-1, 2, l1->opr.op[0], l2->opr.op[0]))->ival == true)
   {
      if (l1->opr.nops == 2 && l2->opr.nops == 2)
         return list_eq(l1->opr.op[1], l2->opr.op[1]);
      else
         return NODE_BOOL_TRUE;
   }

   return NODE_BOOL_FALSE;
}

node *eq(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];
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

node *neq(node *args)
{
   return eq(args)->ival == true ? NODE_BOOL_FALSE : NODE_BOOL_TRUE;
}

node *and(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];

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

node *or(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];

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

node *not(node *args)
{
   node *x = args->opr.op[0];

   ASSERT(x->type, t_bool, "operand to not must be boolean");

   node *ret;

   if (x->ival == true)
      ret = NODE_BOOL_FALSE;
   else
      ret = NODE_BOOL_TRUE;

   decref(x);
   return ret;
}

node *mod(node *args)
{
   node *x = args->opr.op[0];
   node *y = args->opr.op[1];

   ASSERT(x->type, t_int, "left operand to mod must be integer");
   ASSERT(y->type, t_int, "right operand to mod must be integer");

   node *retval = mkint(x->ival % y->ival);

   decref(x);
   decref(y);
   return retval;
}

node *type(node *args)
{
   node *x = args->opr.op[0];
   return mkint(x->type);
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
