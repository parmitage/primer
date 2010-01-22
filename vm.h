#ifndef __QTL_VM_H__
#define __QTL_VM_H__

#include <stdio.h>
#include <string.h>

node* stack[50000];
int stack_ptr;

/* stack operations */
void push(node* node);
node* pop();

/* display objects to stdout */
void display(node* node);
void display_primitive(node* node, int depth);

/* classic Lisp style list operations */
node* car(node* node);
node* cdr(node* node);
node* cons(node* list, node* n);
node* append(node* list1, node* list2);

/* built in operators */
node* add(node* x, node* y);
node* sub(node* x, node* y);
node* mul(node* x, node* y);
node* dvd(node* x, node* y);
node* lt(node* x, node* y);
node* gt(node* x, node* y);
node* lte(node* x, node* y);
node* gte(node* x, node* y);
node* eq(node* x, node* y);
node* neq(node* x, node* y);
node* list_eq(node* l1, node* l2);
node* and(node* x, node* y);
node* or(node* x, node* y);
node* not(node* node);
node* mod(node* x, node* y);

#endif
