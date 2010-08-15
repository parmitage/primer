#ifndef __ARGS_H__
#define __ARGS_H__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int getargs(int argc, char **argv);
void defaults();
void usage();

/* filename to load */
char fname[1000];

/* supress standard library */
int loadlib;

/* supress memory manager */
int memmgr;

/* standard library to load */
char *stdlib;

#endif
