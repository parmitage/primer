#include <stdlib.h>
#include "main.h"
#include "vm.h"
#include "y.tab.h"

void push(node* node)
{
	stack[stack_ptr++] = node;
}

node* pop()
{
	if (stack_ptr > 0)
		return stack[--stack_ptr];
	else
		return mknil();
}

node* car(node* node)
{
	if (node == NULL)
		return mknil();
	else if (node->type == t_nil)
		return node;
	else
		return node->opr.op[0];
}

node* cdr(node* node)
{
	if (node == NULL)
		return mknil();
	else if (node->opr.nops > 0 && node->opr.op[1] != NULL)
	{
		return node->opr.op[1];
	}
	else
		return mknil();
}

node* cons(node* list, node* n)
{
	if (list->type == t_nil)
		return mkcons(LIST, 1, n, NULL);
	else if (n->type == t_nil)
		return mkcons(LIST, 1, list, NULL);
	else
		return mkcons(LIST, 2, n, list);
}

node* append(node* list1, node* list2)
{
	if (list2 == NULL || list2->type == t_nil)
		return list1;

	if (list1 == NULL || list1->type == t_nil)
		return list2;

	node* r = list1;
	node* n = list1;
	
	while (n != NULL && n->type != t_nil && n->opr.op[1] != NULL)
	{
		n = n->opr.op[1];
	}
	
	n->opr.nops = 2;
	n->opr.op[1] = list2;
	
	return r;
}

int node_type(node* node)
{
	return node->type;
}

void display_primitive(node* node, int depth)
{
	switch (node->type)
	{
		case t_int:
			printf("%i", node->ival);
			break;
		case t_float:
			printf("%g", node->fval);
			break;
		case t_nil:
			printf("nil");
			break;
		case t_bool:
			printf("%s", node->ival > 0 ? "true" : "false");
			break;
		case t_char:
			printf("%c", node->ival);
			break;
		
		case t_cons:
		{
			switch (node->opr.oper)
			{
				case STRING:
				{
					while (node != NULL)
					{
						if (node->opr.nops > 0)
						{
							printf("%c", node->opr.op[0]->ival);
							node = node->opr.op[1];
						}
						else
							node = NULL;
					}
					
					break;
				}
				
				case LIST:
				{
					if (depth == 0)
						printf("[");
						
					display_primitive(node->opr.op[0], depth++);
					
					if (node->opr.nops > 1 && node->opr.op[1] != NULL)
					{
						printf(",");
						if (node->opr.op[1]->opr.op[0] != NULL &&
							node->opr.op[1]->opr.op[0]->type == t_cons)
							printf("[");
							
						display_primitive(node->opr.op[1], depth);
					}
					
					if (node->opr.op[1] == NULL)
						printf("]");
							
					--depth;
					
					break;
				}
				
				case LAMBDA:
				{
					printf("LAMBDA");
					break;
				}
			}
			
			break;
		}
		
		case t_symbol:
			printf("%s", node->sval);
			break;
			
		case t_error:
			logerr(node->sval, node	->lineno);
			break;
	}		
}

void display(node* node)
{
	if (node == NULL)
		return;
		
	display_primitive(node, 0);
		
	printf("\n");
}

node* add(node* x, node* y)
{
		if (x->type == t_float)
		{
				if (y->type == t_float)
						return mkfloat(x->fval + y->fval);
				else
						return mkfloat(x->fval + y->ival);
		}
		else
		{
				if (y->type == t_float)
						return mkfloat(x->ival + y->fval);
				else
						return mkint(x->ival + y->ival);
		}		
}

node* sub(node* x, node* y)
{
		if (x->type == t_float)
		{
				if (y->type == t_float)
						return mkfloat(x->fval - y->fval);
				else
						return mkfloat(x->fval - y->ival);
		}
		else
		{
				if (y->type == t_float)
						return mkfloat(x->ival - y->fval);
				else
						return mkint(x->ival - y->ival);
		}		
}

node* mul(node* x, node* y)
{
		if (x->type == t_float)
		{
				if (y->type == t_float)
						return mkfloat(x->fval * y->fval);
				else
						return mkfloat(x->fval * y->ival);
		}
		else
		{
				if (y->type == t_float)
						return mkfloat(x->ival * y->fval);
				else
						return mkint(x->ival * y->ival);
		}		
}

node* dvd(node* x, node* y)
{
	if (x->type == t_float)
	{
		if (y->type == t_float)
			return mkfloat(x->fval / y->fval);
		else
			return mkfloat(x->fval / y->ival);
	}
	else
	{
		if (y->type == t_float)
			return mkfloat(x->ival / y->fval);
		else
			return mkint(x->ival / y->ival);
	}		
}

node* lt(node* x, node* y)
{
		if (x->type == t_float)
		{
				if (y->type == t_float)
					return mkbool(x->fval < y->fval);
				else
					return mkbool(x->fval < y->ival);
		}
		else
		{
				if (y->type == t_float)
					return mkbool(x->ival < y->fval);
				else
					return mkbool(x->ival < y->ival);
		}		
}

node* gt(node* x, node* y)
{
		if (x->type == t_float)
		{
				if (y->type == t_float)
						return mkbool(x->fval > y->fval);
				else
						return mkbool(x->fval > y->ival);
		}
		else
		{
				if (y->type == t_float)
						return mkbool(x->ival > y->fval);
				else
						return mkbool(x->ival > y->ival);
		}		
}

node* lte(node* x, node* y)
{
		if (x->type == t_float)
		{
				if (y->type == t_float)
						return mkbool(x->fval <= y->fval);
				else
						return mkbool(x->fval <= y->ival);
		}
		else
		{
				if (y->type == t_float)
						return mkbool(x->ival <= y->fval);
				else
						return mkbool(x->ival <= y->ival);
		}		
}

node* gte(node* x, node* y)
{
		if (x->type == t_float)
		{
				if (y->type == t_float)
						return mkbool(x->fval >= y->fval);
				else
						return mkbool(x->fval >= y->ival);
		}
		else
		{
				if (y->type == t_float)
						return mkbool(x->ival >= y->fval);
				else
						return mkbool(x->ival >= y->ival);
		}		
}

node* list_eq(node* l1, node* l2)
{
	if (l1 == NULL && l2 == NULL)
		return mkbool(true);
	
	if (l1->type == t_nil && l2->type == t_nil)
		return mkbool(true);

	if (l1->opr.nops != l2->opr.nops)
		return mkbool(false);
	
	if (eq(l1->opr.op[0], l2->opr.op[0])->ival == true)
	{
		if (l1->opr.nops == 2 && l2->opr.nops == 2)
			return list_eq(l1->opr.op[1], l2->opr.op[1]);
		else
			return mkbool(true);
	}
	else
		return mkbool(false);
}

node* eq(node* x, node* y)
{
	if (x->type != y->type)
		return mkbool(false);
		
	if (x->type == t_int || x->type == t_bool)
		return mkbool(x->ival == y->ival);
		
	if (x->type == t_float)
		return mkbool(x->fval == y->fval);

	if (x->type == t_char)
		return mkbool(x->ival == y->ival);
	
	if (x->type == t_nil)
		return mkbool(true);
	
	if (x->type == t_cons)
		return list_eq(x, y);
		
	return mkbool(false);
}

node* neq(node* x, node* y)
{
	return(not(eq(x, y)));
}

node* and(node* x, node* y)
{
	if (x->type == t_bool && y->type == t_bool)
	{
		return mkbool(x->ival && y->ival);
	}
	else
	{
		// TODO probably should throw error
		return mkbool(false);
	}
}

node* or(node* x, node* y)
{
	if (x->type == t_bool && y->type == t_bool)
	{
		return mkbool(x->ival || y->ival);
	}
	else
	{
		// TODO probable should throw error
		return mkbool(false);
	}
}

node* not(node* node)
{
	if (node->ival == true)
		return mkbool(false);
	else
		return mkbool(true);
}

node* mod(node* x, node* y)
{
	if (x->type == t_int && y->type == t_int)
		return mkint(x->ival % y->ival);
	else
		return mkerr("% operator only supports integers", x->lineno);
}

node* range(node* s, node* e)
{
	node* l;

	if (s->type == t_int || e->type == t_int)
	{
		l = mknil();
			
		for (int i = e->ival; i >= s->ival; --i)
			l = cons(l, mkint(i));
	}
	else
	{
		l = mkerr("[..] operator only supports integer ranges", s->lineno);
	}
	
	return l;
}
