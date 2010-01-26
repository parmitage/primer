#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "y.tab.h"

node *con(int value)
{
	node *p;
	size_t nodeSize = sizeof(node) + sizeof(int);
	
	if ((p = malloc(nodeSize)) == NULL)
		abort();
	
	p->type = t_int;
	p->ival = value;
	p->lineno = lineno;
	
	return p;
}

node *fpval(float value)
{
	node *p;
	size_t nodeSize = sizeof(node) + sizeof(float);
	
	if ((p = malloc(nodeSize)) == NULL)
		abort();
	
	p->type = t_float;
	p->fval = value;
	p->lineno = lineno;
	
	return p;
}

node *boolval(int value)
{
	node *p;
	size_t nodeSize = sizeof(node) + sizeof(int);
	
	if ((p = malloc(nodeSize)) == NULL)
		abort();
	
	p->type = t_bool;
	p->ival = value;
	p->lineno = lineno;
	
	return p;
}

node* mkchar(char c)
{
	node* p;
		
	if ((p = malloc(sizeof(node) + sizeof(char))) == NULL)
		abort();

	p->type = t_char;
	p->lineno = lineno;
	p->ival = c;
	
	return p;
}

node* node_make_string(char* value)
{
	int srclen = strlen(value);
	int destlen = srclen - 1;
	int copylen = srclen - 2;
	//char* temp = (char*)malloc(destlen * sizeof(char));
	char temp[1000];
	strncpy(temp, value + 1, copylen);
	temp[copylen] = '\0';
	free(value);
	return node_from_string(temp);
}

node* node_from_string(char* value)
{
	int len = strlen(value);

	if (len > 1)
		return opr(STRING, 2, mkchar(value[0]), node_from_string(value + 1));
	else
		return opr(STRING, 1, mkchar(value[0]));
	
	return nil();
}

node* nil()
{
	node *p;
	size_t nodeSize = sizeof(node) + sizeof(int);
	
	if ((p = malloc(nodeSize)) == NULL)
		abort();
	
	p->type = t_nil;
	p->lineno = lineno;
	
	return p;
}

node *sym(char* s)
{
	node *p;
	size_t nodeSize = sizeof(node) + (strlen(s) * sizeof(char*));
	
	if ((p = malloc(nodeSize)) == NULL)
		abort();
	
	p->type = t_symbol;
	p->sval = strdup(s);
	p->lineno = lineno;
	
	return p;
}

node *opr(int oper, int nops, ...)
{
	if (nops == 0)
		return nil();

	va_list ap;
	node *p;
	int i;
	size_t nodeSize = sizeof(node) + sizeof(oprNodeType) + nops * sizeof(node*);
	
	if ((p = malloc(nodeSize)) == NULL)
		abort();
	
	p->type = t_cons;
	p->lineno = lineno;
	p->opr.oper = oper;
	p->opr.nops = nops;
	
	va_start(ap, nops);
	
	for (i = 0; i < nops; i++)
		p->opr.op[i] = va_arg(ap, node*);
	
	va_end(ap);
	
	return p;
}

void nodefree(node *p)
{
	if (!p)
		return;

	int i;
	
	if (p->type == t_cons)
	{
		for (i = 0; i < p->opr.nops; i++)
			nodefree(p->opr.op[i]);
	}
	
	free (p);
}

void logerr(char* msg, int line)
{
	printf("ERROR (line %i): %s\n", line, msg);
	exit(1);
}

void dbg(char* msg)
{
	printf("%s\n", msg);
}

bool file_exists(const char * path)
{
	FILE *istream;
	
	if((istream = fopen(path, "r")) == NULL)
	{
		return false;
	}
	else
	{
		fclose(istream);
		return true;
	}
}