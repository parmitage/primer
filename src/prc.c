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

      // TODO
      case t_match:
      {
         break;
      }

      case t_lambda:
      case t_closure:
      {
         return Compile_lambda(n);
      }

      case t_cons:
      {
         // n->ast->n1
         // n->ast->n2
         break;
      }

      case t_car:
      {
         //n->ast->n1
         break;
      }

      case t_cdr:
      {
         //n->ast->n1
         break;
      }

      case t_using:
      {
         //n->ast->n1
         break;
      }

      case t_apply:
      {
         /* Compile(n->ast->n1); */

         /* output("("); */

         /* node *params = n->ast->n2; */

         /* while (params != NULL) */
         /* { */
         /*    if (CAR(params)) */
         /*    { */
         /*       Compile(CAR(params)); */
							
         /*       if (CDR(params)) */
         /*          output(", "); */
							
         /*       params = CDR(params); */
         /*    } */
         /*    else */
         /*       params = NULL; */
         /* } */
					
         /* output(")"); */
								
         break;
      }

      case t_cond:
      {            
         /* output("if(");  			 */
         /* Compile(n->ast->n1); */
         /* output(")\n{\n"); */
         /* Compile(n->ast->n2); */
         /* output(";\n}\nelse\n{\n"); */
         /* Compile(n->ast->n3); */
         /* output(";\n}\n"); */
         break;
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
         break;
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
