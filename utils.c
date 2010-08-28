#include <stdio.h>
#include <stdarg.h>
#include "main.h"
#include "utils.h"

bool fexists(const char *path)
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

int args(int argc, char **argv)
{
   opterr = 0;
   int c;
     
   while ((c = getopt(argc, argv, "smtl:")) != -1)
   {
      switch (c)
      {
         case 's':
            arg_loadlib = false;
            break;
         case 'm':
            refctr = true;
            break;
         case 'l':
            arg_stdlib = optarg;
            break;
         case 't':
            arg_trace = true;
            break;
         default:
            return -1;
      }
   }

   /* should be one argument left over for the filename */
   if (optind < argc)
      strcpy(arg_fname, argv[optind]);
   else
      return -1;

   return 0;
}

void defaults()
{
   arg_loadlib = true;
   refctr = false;
   arg_stdlib = "Library";
}

void usage()
{
    printf("usage:\n\tprimer [options] FILENAME\n");
    printf("options:\n");
    printf("\t-s\t\tdon't load standard library\n");
    printf("\t-m\t\tenable GC (experimental)\n");
    printf("\t-t\t\tenable tracing output\n");
    printf("\t-l FILENAME\tload alternative standard library\n");
}

void trace(const char *fmt, ...)
{
   if (arg_trace == true)
   {
      va_list args;
      va_start(args, fmt);
      vfprintf(stdout, fmt, args);
      va_end(args);
      fprintf(stdout, "\n");
   }
}
