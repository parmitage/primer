#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "gc.h"

void GC_init()
{
   //printf("extending heap\n");

   for (int i = 0; i < GC_PREALLOC_SIZE; ++i)
   {
      node *n;

      if ((n = (struct node*)malloc(sizeof(struct node))) == NULL)
         error("unable to allocate memory");

      n->hist = freelist;
      freelist = n;
   }
}

node *GC_static_alloc()
{
   //printf("GC_static_alloc\n");

   node *n;

   if ((n = (struct node*)malloc(sizeof(struct node))) == NULL)
      error("unable to allocate memory");

   return n;
}

void GC_disable()
{
   gc_enabled = false;
}

void GC_enable()
{
   gc_enabled = true;
}

node *GC_alloc()
{
   /* TODO if we want to intercept a setting for disabling the GC then here
      we could check and call GC_static_alloc and return */

   if (!gc_enabled)
   {
      return GC_static_alloc();
   }

   if (GC_heap_exhausted())
   {
      GC_collect();

      if (GC_heap_exhausted())
      {
         GC_init();

         if (GC_heap_exhausted())
            error("unable to extend the heap");
      }
   }

   /* we have free space so allocate from the free list */
   node *temp = freelist;
   freelist = freelist->hist;
   temp->hist = current;
   current = temp;

   //printf("allocating: ");
   //pprint(temp);
   //printf("\n"); fflush(stdout);

   /* The mark bit is initially set to -1 meaning that the memory isn't
      eligible for collection. Memory is only marked as eligible for
      collection once it has been interned either through a definition
      or through binding function arguments. This means that some memory
      can never be freed - the question is how much? */
   temp->mark = -1;

   printf("GC_alloc\n");

   return temp;
}

void GC_free(node *n)
{
   if (n->mark < 0)
      return;

   /* switch (n->type) */
   /* { */
   /*    case t_closure: */
   /*       free(n->fn); */
   /*       break; */

   /*    case t_pair: */
   /*       free(n->pair); */
   /*       break; */
   /* } */

   /* move the node to the head of the free list */
   n->hist = freelist;
   freelist = n;
}

void GC_mark_roots(env *env)
{
   struct env *e = env;

   if (e == NULL)
   {
      printf("environment is NULL!\n");
      fflush(stdout);
   }

   while (e != NULL)
   {
      binding *b = e->bind;

      while (b != NULL)
      {
         //printf("mark_roots:");
         //pprint(b->node);
         //printf("\n");

         GC_recursive_markbit_set(b->node, true);
         b = b->prev;
      }

      e = e->parent;
   }
}

void GC_collect()
{
   printf("GC_collect\n");

   if (!gc_enabled)
      return;

   GC_mark_roots(cenv);

   node *last = current;
   SCAV = GC_get_prev(last);
   
   while (SCAV != first)
   {
      if (SCAV->mark == 1)
      {
         GC_recursive_markbit_set(SCAV, 1);
         GC_markbit_set(SCAV, 0);
         last = SCAV;
         SCAV = GC_get_prev(last);
      }
      else
      {
         node *tmp = SCAV;

         if (tmp->mark < 0)
         {
            //printf(".");
            SCAV = GC_get_prev(SCAV);
         }
         else
         {
            printf("collecting: ");
            pprint(tmp);
            printf("\n"); fflush(stdout);
            
            SCAV = GC_get_prev(SCAV);
            GC_relink_nodes(last, SCAV);
            GC_free(tmp);
         }
      }
   }
}

void GC_recursive_markbit_set(node *n, bool m)
{
   if (!gc_enabled)
      return;

   if (n == NULL)
      return;

   GC_markbit_set(n, m);

   switch (n->type)
   {
      case t_int:
      case t_float:
      case t_bool:
      case t_char:
      case t_symbol:
         break;

      case t_val:
      {
         GC_markbit_set(n->ast->n1, m);
         GC_recursive_markbit_set(n->ast->n2, m);
         break;
      }

      case t_lambda:
      {
         GC_recursive_markbit_set(n->fn->args, m);
         GC_recursive_markbit_set(n->fn->body, m);
         break;
      }

      case t_closure:
      {
         GC_recursive_markbit_set(n->fn->args, m);
         GC_recursive_markbit_set(n->fn->body, m);
         
         binding *b = n->fn->env->bind;
         //printf("GC_recursive_mark: iterate over bindings\n"); fflush(stdout);

         while (b != NULL)
         {
            printf(".");
            GC_markbit_set(b->node, 1);
            b = b->prev;
         }

         break;
      }

      case t_apply:
      {
         GC_recursive_markbit_set(n->ast->n1, m);
         GC_recursive_markbit_set(n->ast->n2, m);
         break;
      }

      case t_pair:
      {
         struct node *iter = n;
         GC_markbit_set(iter->pair->car, m);
         GC_recursive_markbit_set(iter->pair->cdr, m);
         break;
      }
   }
}

void GC_markbit_set(node *n, bool m)
{
   if (!gc_enabled)
      return;

   if (n != NULL)
   {
      //printf("marking: ");
      //pprint(n);
      //printf("\n"); fflush(stdout);

      if (m >= 0 && n->mark >= -1)
         n->mark = m;
   }
}

/* void GC_unmark(node *n) */
/* { */
/*    if (n != NULL && n->mark != -1) */
/*       n->mark = 0; */
/* } */

node *GC_get_prev(node *n)
{
   if (n != NULL)
      return n->hist;
}

void GC_relink_nodes(node *n1, node *n2)
{
   if (n1 != NULL)
      n1->hist = n2;
}

bool GC_heap_available()
{
   return freelist != NULL;
}

bool GC_heap_exhausted()
{
   return freelist == NULL;
}

