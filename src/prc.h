#ifndef __PRIMER_PRC_H__
#define __PRIMER_PRC_H__

#include <stdio.h>
#include "main.h"

const int MAX_ARGS = 20;

char *outfilename;
FILE *outfile;

void Compiler_init(int argc, char **argv);
char *Compile_file(char *fname);
void Compiler_output(char* str);
char *Compile(node *n);

char *Compile_int(node *n);
char *Compile_float(node *n);
char *Compile_bool(node *n);
char *Compile_char(node *n);
char *Compile_string(node *n);
char *Compile_symbol(node *n);
char *Compile_val(node *n);
char *Compile_let(node *n);
char *Compile_lambda(node *n);
char *Compile_using(node *n);
char *Compile_apply(node *n);
char *Compile_cons(node *n);
char *Compile_car(node *n);
char *Compile_cdr(node *n);
char *Compile_cond(node *n);
char *Compile_seq(node *n);
char *Compile_pair(node *n);
char *Compile_binop(node *n, char *op);
char *Compile_uniop(node *n, char *op);
char *Compile_append(node *n);
char *Compile_range(node *n);
char *Compile_at(node *n);
char *Compile_is(node *n);
char *Compile_as(node *n);
char *Compile_show(node *n);
char *Compile_len(node *n);
char *Compile_rnd(node *n);
char *Compile_eq(node *n);
char *Compile_neq(node *n);

#endif
