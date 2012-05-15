#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prc.h"
#include "main.h"
#include "pstring.h"
#include "y.tab.h"

int main(int argc, char **argv)
{
   if (argc != 2)
   {
      printf("usage: prc prog.pri out.js\n");
      return -1;
   }

   //Compiler_init(argc, argv);
   node *n = parse(argv[1]);
   Compiler_output(Compile(n));

   return 0;
}

void Compiler_init(int argc, char **argv)
{
   outfilename = argv[2];
   outfile = fopen(outfilename, "w");

   if (outfile == NULL)
   {
      // TODO log error   
      return;
   }

   //NODE_BOOL_TRUE = mkbool(true);
   //NODE_BOOL_FALSE = mkbool(false);
   //NODE_ANY = mkbool(-1);

   intern("newline");
   intern("tab");
   intern("int");
   intern("float");
   intern("bool");
   intern("char");
   intern("list");
   intern("string");
   intern("lambda");
}

void Compiler_output(char* str)
{
   //fputs(str, outfile);
   printf("%s\n", str);
   fflush(stdout);
}

char *Compile(node *n)
{
   static bool infun = false;

   switch (n->type)
   {
      case t_int:
      {
         return Compile_int(n);
      }
		
      case t_float:
      {
         return Compile_float(n);
      }
		
      case t_bool:
      {
         return Compile_bool(n);
      }
		
      case t_char:
      {
         return Compile_char(n);
      }
		
      case t_string:
      {
         return Compile_string(n);
      }
		
      case t_symbol:
      {
         return Compile_symbol(n);
      }

      case t_val:
      {
         return Compile_val(n);
      }

      case t_let:
      {
         return Compile_let(n);
      }

      case t_match:
      {
         // TODO
         break;
      }

      case t_lambda:
      {
         return Compile_lambda(n);
      }

      case t_using:
      {
         // TODO
         break;
      }

      case t_apply:
      {
         // TODO
         break;
      }

      case t_cond:
      {            
         return Compile_cond(n);
      }

      case t_seq:
      {
         char *s1 = Compile(n->ast->n1);

         if (n->ast->n2 != NULL)
         {
            char *s2 = Compile(n->ast->n2);
            int sz = snprintf(NULL, 0, "%s\n%s", s1, s2);
            char *p = (char*)malloc(sz + 1);
            sprintf(p, "%s\n%s", s1, s2);
            return p;
         }
         
         return s1;
      }

      case t_pair:
      {
         // TODO
         break;
      }

      /**********************************************************************
                                binary operators
      **********************************************************************/

      case t_add:
      {
         return Compile_binop(n, "+");
      }

      case t_sub:
      {
         return Compile_binop(n, "-");
      }

      case t_mul:
      {
         return Compile_binop(n, "*");
      }

      case t_dvd:
      {
         return Compile_binop(n, "/");
      }

      case t_lt:
      {
         return Compile_binop(n, "<");
      }

      case t_gt:
      {
         return Compile_binop(n, ">");
      }

      case t_gte:
      {
         return Compile_binop(n, ">=");
      }

      case t_lte:
      {
         return Compile_binop(n, "<=");
      }

      case t_neq:
      {
         return Compile_neq(n);
      }

      case t_eq:
      {
         return Compile_eq(n);
      }

      case t_and:
      {
         return Compile_binop(n, "&&");
      }

      case t_or:
      {
         return Compile_binop(n, "||");
      }

      case t_b_and:
      {
         return Compile_binop(n, "&");
      }

      case t_b_or:
      {
         return Compile_binop(n, "|");
      }

      case t_b_xor:
      {
         return Compile_binop(n, "^");
      }

      case t_b_lshift:
      {
         return Compile_binop(n, "<<");
      }

      case t_b_rshift:
      {
         return Compile_binop(n, ">>");
      }

      case t_mod:
      {
         return Compile_binop(n, "%");
      }

      case t_append:
      {
         return Compile_append(n);
      }

      case t_range:
      {
         return Compile_range(n);
      }

      case t_at:
      {
         return Compile_at(n);
      }

      case t_as:
      {
         // TODO
         break;
      }

      case t_is:
      {
         // TODO
         break;
      }

      case t_cons:
      {
         return Compile_cons(n);
      }

      /**********************************************************************
                                unary operators
      **********************************************************************/

      case t_car:
      {
         return Compile_car(n);
      }

      case t_cdr:
      {
         return Compile_cdr(n);
      }

      case t_neg:
      {
         return Compile_uniop(n, "-");
      }

      case t_not:
      {
         return Compile_uniop(n, "!");
      }

      case t_bnot:
      {
         return Compile_uniop(n, "~");
      }

      case t_show:
      {
         return Compile_show(n);
      }

      case t_reads:
      {
         // TODO
         break;
      }

      case t_len:
      {
         return Compile_len(n);
      }

      case t_rnd:
      {
         return Compile_rnd(n);
      }
   }
}

char *Compile_int(node *n)
{
   int sz = snprintf(NULL, 0, "%i", n->ival);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "%i", n->ival);
   return p;
}

char *Compile_float(node *n)
{
   int sz = snprintf(NULL, 0, "%gM", n->fval);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "%gM", n->fval);
   return p;
}

char *Compile_bool(node *n)
{
   char *temp = (char*)malloc(6);
   sprintf(temp, "%s", n->ival <= 0 ? "false" : "true");
   return temp;
}

char *Compile_char(node *n)
{
   int sz = snprintf(NULL, 0, "%c", n->ival);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "%c", n->ival);
   return p;
}

char *Compile_string(node *n)
{
   char *str = node_to_str(n);
   int sz = snprintf(NULL, 0, "\"%s\"", str);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "\"%s\"", str);
   return p;
}

char *Compile_symbol(node *n)
{
   char *str = symname(n->ival);
   int sz = snprintf(NULL, 0, "%s", str);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "%s", str);
   return p;
}

char *Compile_val(node *n)
{
   char *lhs = Compile(n->ast->n1);
   char *rhs = Compile(n->ast->n2);
   int sz = snprintf(NULL, 0, "var %s = %s;", lhs, rhs);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "var %s = %s;", lhs, rhs);
   return p;
}

char *Compile_let(node *n)
{
   node *bindings = n->ast->n1;
   char *bindings_str = (char*)malloc(16);
   bindings_str = strdup("(function () {\n");

   while (bindings != NULL)
   {
      node *expression = bindings->ast->n1;
      char *lhs = Compile(expression->ast->n1);
      char *rhs = Compile(expression->ast->n2);

      int sz = snprintf(NULL, 0, "    var %s = %s;\n", lhs, rhs);
      char *p = (char*)malloc(sz + 1);
      sprintf(p, "    var %s = %s;\n", lhs, rhs);

      int sz2 = snprintf(NULL, 0, "%s%s", bindings_str, p);
      char *temp = (char*)malloc(sz2 + 1);
      sprintf(temp, "%s%s", bindings_str, p);

      strcpy(bindings_str, temp);

      free(p);
      free(temp);
      //free(lhs);
      //free(rhs);

      bindings = bindings->ast->n2;
   }

   char *body = Compile(n->ast->n2);
   int sz3 = snprintf(NULL, 0, "%s    return %s;\n})()\n", bindings_str, body);
   char *ret = (char*)malloc(sz3 + 1);
   sprintf(ret, "%s    return %s;\n})()\n", bindings_str, body);

   return ret;
}

char *Compile_lambda(node *n)
{
   node *params = n->ast->n1;
   char *params_arr[20];   /* TODO add global constant MAX_ARGS */
   int param_count = 0;

   while (params != NULL)
   {
      params_arr[param_count++] = Compile(CAR(params));
      params = CDR(params);
   }

   char *params_str = PString_intersperse(params_arr, param_count, ", ");
   char *body = Compile(n->ast->n2);

   int sz = snprintf(NULL, 0, "function (%s) {\n    return%s;\n}", params_str, body);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "function (%s) {\n    return %s;\n}", params_str, body);

   return p;
}

char *Compile_cons(node *n)
{
   char *lhs = Compile(n->ast->n1);
   char *rhs = Compile(n->ast->n2);

   int sz = snprintf(NULL, 0, "cons(%s, %s)", lhs, rhs);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "cons(%s, %s)", lhs, rhs);

   return p;
}

char *Compile_car(node *n)
{
   char *e = Compile(n->ast->n1);

   int sz = snprintf(NULL, 0, "head(%s)", e);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "head(%s)", e);

   return p;
}

char *Compile_cdr(node *n)
{
   char *e = Compile(n->ast->n1);

   int sz = snprintf(NULL, 0, "%s.slice(1)", e);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "%s.slice(1)", e);

   return p;
}

char *Compile_cond(node *n)
{
   char *pred = Compile(n->ast->n1);
   char *cond = Compile(n->ast->n2);
   char *alt = Compile(n->ast->n3);

   int sz = snprintf(NULL, 0, "(%s) ? (%s) : (%s)", pred, cond, alt);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "(%s) ? (%s) : (%s)", pred, cond, alt);

   return p;
}

char *Compile_binop(node *n, char *op)
{
   char *lhs = Compile(n->ast->n1);
   char *rhs = Compile(n->ast->n2);

   int sz = snprintf(NULL, 0, "(%s %s %s)", lhs, op, rhs);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "(%s %s %s)", lhs, op, rhs);

   return p;
}

char *Compile_uniop(node *n, char *op)
{
   char *exp = Compile(n->ast->n1);

   int sz = snprintf(NULL, 0, "%s%s", op, exp);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "%s%s", op, exp);

   return p;
}

char *Compile_append(node *n)
{
   char *lhs = Compile(n->ast->n1);
   char *rhs = Compile(n->ast->n2);

   int sz = snprintf(NULL, 0, "%s.concat(%s)", lhs, rhs);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "%s.concat(%s)", lhs, rhs);

   return p;
}

char *Compile_range(node *n)
{
   int from = n->ast->n1->ival;
   int to = n->ast->n2->ival;

   int sz = snprintf(NULL, 0, "range(%i, %i)", from, to);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "range(%i, %i)", from, to);

   return p;
}

char *Compile_at(node *n)
{
   char *lhs = Compile(n->ast->n1);
   char *rhs = Compile(n->ast->n2);

   int sz = snprintf(NULL, 0, "%s[%s]", lhs, rhs);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "%s[%s]", lhs, rhs);

   return p;
}

char *Compile_show(node *n)
{
   char *exp = Compile(n->ast->n1);

   int sz = snprintf(NULL, 0, "document.write(%s + \"<br/>\");", exp);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "document.write(%s + \"<br/>\");", exp);

   return p;
}

char *Compile_len(node *n)
{
   char *exp = Compile(n->ast->n1);

   int sz = snprintf(NULL, 0, "%s.length", exp);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "%s.length", exp);

   return p;
}

char *Compile_rnd(node *n)
{
   int x = n->ast->n1->ival + 1;

   int sz = snprintf(NULL, 0, "Math.floor(Math.random() * %i)", x);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "Math.floor(Math.random() * %i)", x);

   return p;
}

char *Compile_eq(node *n)
{
   char *lhs = Compile(n->ast->n1);
   char *rhs = Compile(n->ast->n2);

   int sz = snprintf(NULL, 0, "equals(%s, %s)", lhs, rhs);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "equals(%s, %s)", lhs, rhs);

   return p;
}

char *Compile_neq(node *n)
{
   char *lhs = Compile(n->ast->n1);
   char *rhs = Compile(n->ast->n2);

   int sz = snprintf(NULL, 0, "!equals(%s, %s)", lhs, rhs);
   char *p = (char*)malloc(sz + 1);
   sprintf(p, "!equals(%s, %s)", lhs, rhs);

   return p;
}
