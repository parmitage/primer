#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pstring.h"

char* PString_join(char *strings[], int count)
{
   char* str = NULL;
   size_t len = 0;

   for (int i = 0 ; i < count ; ++i)
      len += strlen(strings[i]);
   
   ++len; /* NULL terminator */

   str = (char*)malloc(len);
   str[0] = '\0';

   for (int i = 0; i < count; ++i)
      strcat(str, strings[i]);

   return str;
}

char* PString_intersperse(char *strings[], int count, char *delim)
{
   if (count == 0)
      return "";

   char* str = NULL;
   size_t len = 0;
   size_t delim_len = strlen(delim);

   for (int i = 0 ; i < count ; ++i)
      len += strlen(strings[i]);
      
   len += delim_len * (count - 1);
   ++len; /* NULL terminator */

   str = (char*)malloc(len);
   str[0] = '\0';

   for (int i = 0; i < count; ++i)
   {
      strcat(str, strings[i]);
      
      if (i + 1 < count)
         strcat(str, delim);
   }

   return str;
}
