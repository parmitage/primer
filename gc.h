#ifndef __GC_H__
#define __GC_H__

#include <stdlib.h>
#include "main.h"

#define GC_PREALLOC_SIZE 500

node *freelist;
node *SCAV;
node *first;       /* first allocated cell */
node *current;     /* last allocated cell */
bool gc_enabled;
extern env *cenv;  /* current environment defined in eval.h */
extern env *top;

void GC_init();
void GC_disable();
void GC_enable();
node *GC_alloc();
node *GC_static_alloc();
void GC_free(node *n);
void GC_mark_roots(env *env);
void GC_collect();
bool GC_is_marked(node *n);
void GC_recursive_markbit_set(node *n, bool m);
void GC_markbit_set(node *n, bool m);
node *GC_get_prev(node *n);
void GC_relink_nodes(node *n1, node *n2);
bool GC_heap_available();
bool GC_heap_exhausted();

#endif
