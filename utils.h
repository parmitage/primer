#ifndef __PRIMER_UTILS_H__
#define __PRIMER_UTILS_H__

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool fexists(const char * filename);
int args(int argc, char **argv);
void defaults();
void usage();
void trace(const char *fmt, ...);

#endif
