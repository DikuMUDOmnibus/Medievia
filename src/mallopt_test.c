#include <stdio.h>
#include <malloc.h>


main()
{
char *a=NULL,*b=NULL;
    malloc_debug(1);
    a=malloc(0);
    strcpy(a,"test");
    if(a[0])exit(0);
}
