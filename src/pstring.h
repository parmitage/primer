#ifndef __PRIMER_PSTRING__
#define __PRIMER_PSTRING__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* PString_join(char *strings[], int count);
char* PString_intersperse(char *strings[], int count, char *delim);

#endif
