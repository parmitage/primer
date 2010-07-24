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
  // initialise SS1 to one memory page

  // get the current environment

  // walk the current environment up to the root and for each environment:
  // 1. mark it as locked
  // 2. for each symbol in the environment, copy it to SS1 (extending pages as required)
  // 3. 

  // iterate over all environments from the env_list and for each:
  // if the environment isn't locked, free it
  // else, unlock it and copy it to live_env_list
  // copy live_env_list head pointer to env_list

  // set current_semi_space to SS1
  
  // free memory in SS0 and set SS0 pointers to start of block
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
