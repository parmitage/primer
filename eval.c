#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "vm.h"
#include "eval.h"
#include "y.tab.h"

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("usage: pl0 <filename>\n");
		return -1;
	}
	
	node* stdlib = load_std_lib();
	lineno = 1;
	parse(argv[1]);
	node* program = create_program(stdlib, ast);
	eval(program, NULL);
	
	return 0;
}

node* load_std_lib()
{
	char* libpath;
  	libpath = getenv("PRIMER_LIBRARY_PATH");
  	
  	if (libpath == NULL)
  	{
    	printf("The environment variable PRIMER_LIBRARY_PATH has not been set\n");
    	exit(-1);
    }
    
    if (!file_exists(libpath))
    {
    	printf("Unable to find standard library at %s\n", libpath);
    	exit(-1);
    }
	
	parse(libpath);
	return ast->opr.op[0];
}

node* create_program(node* stdlib, node* user)
{
	node* level1 = mkcons(';', 2, stdlib, ast->opr.op[0]);
	return mkcons(PROG, 1, level1);
}

void eval(node *p, environment* env)
{
	if (!p)
		return;

	switch(p->type)
	{
		case t_int:
		case t_float:
		case t_bool:
		case t_char:
		case t_nil:
			push(p);
			break;

		case t_error:
		{
			logerr(p->sval, p->lineno);
			break;
		}

		case t_symbol:
		{
			binding *b = environment_lookup(env, p->sval);
			
			if (b != NULL)
				eval(b->node, env);
			else
				logerr("unbound symbol", p->lineno);
		}

		case t_cons:
		{
			switch(p->opr.oper)
			{
				case PROG:
				{
					/* create the global lexical environment */
					env = environment_new(NULL);
					eval(p->opr.op[0], env);
					env = environment_delete(env);
					break;
				}
				
				case DEF:
				{
					char* name = p->opr.op[0]->sval;
					eval(p->opr.op[1], env);
					binding* binding = binding_new(name, pop());
					environment_extend(env, binding);
					break;
				}
				
				case LAMBDA:
				{
					/*	Direct evaluation of lambda, presumably when passed as
						an anonymous function to another function rather than
						via a funcall, means pushing the lambda back onto the
						stack ready to be evaluated. */
					push(p);
					break;
				}
				
				case FUNCALL:
				{
					/* evaluate the expression (which could be a symbol or something returning a function) */
					eval(p->opr.op[0], env);

					/* pop the function off the stack */
					node* fn = pop();

					/* TODO check that fn is a LAMBDA */
					/* TODO check correct number of arguments are passed to function */
					/* TODO could more efficiently bind the parameters now that FUNCALL
						actually evaluates the lambda itself. At the moment this is done
						in two steps:
							1. evaluating the supplied parameters
							2. binding the parameters to the lambda list
					*/
					
					/*	recurse over the parameters, evaluating them in turn
						thus ultimately pushing their value onto the stack */
					node *params = p->opr.op[1];

					while (params != NULL)
					{
						if (params->opr.nops > 0)
						{
							eval(params->opr.op[0], env);
							params = params->opr.op[1];
						}
						else
							params = NULL;
					}
				
					/* now evaluate the lambda */					
					environment* ext = environment_new(env);

					/* pop the values and bind them to the function parameters */
					bind(fn->opr.op[0], ext);

					/* evaluate the function body */
					eval(fn->opr.op[1], ext);
					
					environment_delete(ext);

					break;
				}
				
				case LIST:
				{
					// Because we can store symbols in lists we evaluate the
					// list contents. Probably could make this significantly
					// more efficient!
					switch (p->opr.nops)
					{
						case 0:
							push(mkcons(LIST, 0));
							break;
						case 1:
							eval(p->opr.op[0], env);
							push(mkcons(LIST, 1, pop()));
							break;
						case 2:
							eval(p->opr.op[0], env);
							eval(p->opr.op[1], env);
							push(mkcons(LIST, 2, pop(), pop()));
							break;
					}
					break;
				}
				
				case STRING:
				{
					push(p);
					break;
				}
				
				case SHOW:
				{
					eval(p->opr.op[0], env);
					node* val = pop();
					display(val);
					break;
				}
				
				case HEAD:
				{
					eval(p->opr.op[0], env);
					node* list = pop();
					push(car(list));
					break;	
				}
				
				case TAIL:
				{
					eval(p->opr.op[0], env);
					node* list = pop();
					push(cdr(list));
					break;
				}
				
				case CONS:
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(cons(pop(), pop()));
					break;
				}
				
				case ';':
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					break;
				}
				
				case IF:
				{
					eval(p->opr.op[0], env);					
					node *pred = pop();
					
					if (pred->ival > 0)
					{
						eval(p->opr.op[1], env);
					}
					else if (p->opr.nops > 3)
					{
						eval(p->opr.op[2], env);
					}
					
					break;
				}
				
				case '+':
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(add(pop(), pop()));
					break;
				}
				
				case '-':
				{
					if (p->opr.nops == 1)
					{
						eval(p->opr.op[0], env);
						push(sub(mkint(0), pop()));
					}
					else if (p->opr.nops == 2)
					{
						eval(p->opr.op[0], env);
						eval(p->opr.op[1], env);
						push(sub(pop(), pop()));
					}

					break;
				}
				
				case '*':
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(mul(pop(), pop()));
					break;
				}
				
				case '/':
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(dvd(pop(), pop()));
					break;
				}
				
				case '<':
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(lt(pop(), pop()));
					break;
				}
				
				case '>':
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(gt(pop(), pop()));
					break;
				}
				
				case GE:
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(gte(pop(), pop()));
					break;
				}
				
				case LE:
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(lte(pop(), pop()));
					break;
				}
				
				case NE:
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(neq(pop(), pop()));
					break;	
				}
				
				case EQ:
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(eq(pop(), pop()));
					break;
				}
				
				case AND:
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(and(pop(), pop()));
					break;
				}
				
				case OR:
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(or(pop(), pop()));
					break;
				}
				
				case MOD:
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(mod(pop(), pop()));
					break;
				}
				
				case APPEND:
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(append(pop(), pop()));
					break;
				}
				
				case RANGE:
				{
					eval(p->opr.op[0], env);
					eval(p->opr.op[1], env);
					push(range(pop(), pop()));
					break;
				}
				
				case NOT:
				{
					eval(p->opr.op[0], env);
					push(not(pop()));
					break;
				}
				
				case TYPE:
				{
					int t = node_type(p->opr.op[0]);
					
					if (t == t_symbol)
					{
						binding *b = environment_lookup(env, p->opr.op[0]->sval);
						
						if (b != NULL)
							t = b->node->type;
						else
							t = -1;
					}

					push(mkint(t));
					break;
				}
			}
		}
	}
}

void bind(node* params, environment* env)
{
	if (params != NULL)
	{		
		if (params->opr.nops > 1)
		{
			bind(params->opr.op[1], env);
		}
		
		if (params->opr.nops > 0)
		{
			binding* binding = binding_new(params->opr.op[0]->sval, pop());
			environment_extend(env, binding);
		}
	}
}

binding* binding_new(char* name, node* node)
{
	size_t size = sizeof(binding) + (strlen(name) * sizeof(char)) + sizeof(node);
	binding* bind = (binding*)malloc(size);
	
	bind->name = strdup(name);
	bind->node = node;
	
	return bind;
}

environment* environment_new(environment* enclosing)
{
	size_t size = sizeof(environment) + MAX_BINDINGS_PER_FRAME * sizeof(binding*);
	environment* env = (environment*)malloc(size);
	
	/* enclosing environment will be NULL for global environment */
	env->enclosing = enclosing;
	env->count = 0;
	
	return env;
}

environment *environment_delete(environment* env)
{
	environment *enclosing = env->enclosing;
	
	/*	note that we free the string allocated to hold the binding name and
		the pointer itself but we DO NOT free the node that the binding
		points to as this is a reference into the AST */
	
	for (int i = 0; i < env->count; ++i)
	{
		free(env->bindings[i]->name);
	}
	
	free(env);
	
	return enclosing;
}

void environment_extend(environment* env, binding *binding)
{
	env->bindings[env->count] = binding;
	env->count++;
}

binding* environment_lookup(environment* env, char* name)
{
	if (env != NULL)
	{
		for (int i = 0; i < env->count; ++i)
		{
			if (strcmp(env->bindings[i]->name, name) == 0)
			{
				return env->bindings[i];
			}
		}
		
		return environment_lookup(env->enclosing, name);
	}
	
	return NULL;
}
