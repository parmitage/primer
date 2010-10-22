#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "utils.h"
#include "eval.h"
#include "y.tab.h"

/*
  TODO redesign cons cells (and rename to t_cons)
  TODO separate out ast nodes from runtime data structures (closures, lists, etc)?
  TODO hand written parser
  TODO REPL
  TODO reference counter
  TODO proper tail recursion checks
  TODO proper build closure environment
  TODO make def an expression rather than a statement?
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
   extend(top, bindnew(intern("string"), mkint(t_string)));

   lineno = 1;

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

      case t_def:
      {
         extend(e, bindnew(n->ast->n1->ival, eval(n->ast->n2, e)));
         break;
      }

      case t_lambda:
      {
         //env *ce = envnew(top);
         //build_closure_env(n->opr.op[1], env, ce);
         return mkclosure(n->ast->n1, n->ast->n2, n->ast->n3, e);
      }

      case t_operator:
      {
         /* TODO this use of mkpair would be tidied up when we have proper cons cells */
         return n->op->primitive(mkpair(-1, 2, eval(n->op->arg1, e), eval(n->op->arg2, e)));
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
            //e = envdel(ext);
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
         switch(n->opr.oper)
         {
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
         }
      }
   }
}

void bind(node *args, node *params, env *fnenv, env *argenv)
{
   if (params != NULL)
   {		
      if (params->opr.nops > 1)
         bind(cdr(args), cdr(params), fnenv, argenv);
		
      if (params->opr.nops > 0)
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
   return str_to_node(temp);
}

node* str_to_node(char* value)
{
   int len = strlen(value);

   if (len > 1)
      return mkpair(STRING, 2, mkchar(value[0]), str_to_node(value + 1));
   else
      return mkpair(STRING, 1, mkchar(value[0]));
}

char *node_to_str(node *node)
{
   char *str = (char*)malloc(50);
   int i = 0;

   while (node != NULL)
   {
      if (node->opr.nops > 0)
      {
         str[i] = node->opr.op[0]->ival;
         node = node->opr.op[1];
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

node *at(node *args)
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

node *as(node *args)
{
   node *from = args->opr.op[0];
   node *to = args->opr.op[1];
   int target = to->ival;
   char buffer[10];
   int ival;
   float fval;

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
               error("conversion from float to int not supported");

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
         if (from->opr.oper == STRING)
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
