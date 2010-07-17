#include <stdio.h>
#include <stdlib.h>
#include "gc.h"

void *gcinit()
{
  allocbuf = malloc(ALLOCSIZE);
  allocp = allocbuf;
}

void *gcfree()
{

}

void gcrealloc()
{
  /* try to allocate another page, space permitting */
  if (ALLOCSIZE < OVERALLOC)
    {
      ++pages;
      allocbuf = (void *)realloc(allocbuf, ALLOCSIZE);

      if (allocbuf == NULL)
        {
          printf("unable to extend managed heap\n");
          exit(1);
        }

      allocp = allocbuf + totalloc;
    }
  else
    {
      printf("out of memory\n");
      printf("%c%c%c%c%c\n", ((char*)allocbuf)[0], ((char*)allocbuf)[1],
             ((char*)allocbuf)[2], ((char*)allocbuf)[3], ((char*)allocbuf)[4]);
      exit(1);
    }
}

void *gcalloc(size_t n)
{
  if (FREESPACE < n)
    {
      /* we need to collect */
      gcfree();
      
      /* check if enough free memory after collection */
      if (FREESPACE <= LOWLIMIT)
        gcrealloc();      
    }
  
  allocp += n;
  totalloc += n;
  return allocp - n;
}
