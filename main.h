#ifndef __PRIMER_MAIN_H__
#define __PRIMER_MAIN_H__

typedef enum { false = 0, true = 1 } bool;

#define when(pred, expr) if (pred) expr

/* command line arguments to the interpreter */
char arg_fname[1000];
bool arg_loadlib;
bool refctr;
char *arg_stdlib;
bool arg_trace;

#endif
