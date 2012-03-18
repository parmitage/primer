#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"

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

node *prialloc()
{
   node *n;

   if ((n = (struct node*)malloc(sizeof(struct node))) == NULL)
      error("unable to allocate memory");

   return n;
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
