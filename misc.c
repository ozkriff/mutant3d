#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include "bool.h"

/*Compare strings till first space or tab.
  strcmp_sp("lo 2 3", "lo %d %d") -> true
  strcmp_sp("no 2 3", "lo %d %d") -> false */
bool strcmp_sp (const char *s1, const char *s2){
  while(s2 && s1){
    if(*s1 != *s2)
      return(false);
    if(*s1==' ' || *s1=='\t')
      return(true);
    s1++, s2++;
  }
  return(true);
}

char *my_strdup (const char *s){
    char *d = malloc(strlen(s) + 1);
    if(d)
        strcpy(d, s);
    return(d);
}

void die (const char *errstr, ...){
  va_list l;
  va_start(l, errstr);
  vfprintf(stderr, errstr, l);
  va_end(l);
  exit(EXIT_FAILURE);
}
