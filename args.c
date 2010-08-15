#include "args.h"

int getargs(int argc, char **argv)
{
   opterr = 0;
   int c;
     
   while ((c = getopt(argc, argv, "sml:")) != -1)
   {
      switch (c)
      {
         case 's':
            loadlib = 0;
            break;
         case 'm':
            memmgr = 0;
            break;
         case 'l':
            stdlib = optarg;
            break;
         default:
            return -1;
      }
   }

   /* should be one argument left over for the filename */
   if (optind < argc)
      strcpy(fname, argv[optind]);
   else
      return -1;

   return 0;
}

void defaults()
{
   loadlib = 1;
   memmgr = 1;
   stdlib = "Library";
}

void usage()
{
    printf("usage:\n\tprimer [options] FILENAME\n");
    printf("options:\n");
    printf("\t-s\t\tdon't load standard library\n");
    printf("\t-m\t\tdisable memory manager\n");
    printf("\t-l FILENAME\tload alternative standard library\n");
}
