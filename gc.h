#ifndef __PRIMER_GC_H__
#define __PRIMER_GC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *allocbuf;
static void *allocp;
static int pages = 1;
static int totalloc = 0;

#define MAXPAGES 6
#define ALLOCSIZE 1024 * 1024 * 100 * pages
#define FREESPACE allocbuf + ALLOCSIZE - allocp
#define LOWLIMIT 1024 * 1024 * 5
#define OVERALLOC 1024 * 1024 * 100 * MAXPAGES

void *gcinit();
void *gcfree();
void gcrealloc();
void *gcalloc(size_t n);

#endif
