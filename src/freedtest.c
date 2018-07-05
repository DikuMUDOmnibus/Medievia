#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <ctype.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"

void *my_free(
void *ptr
)
{

    if (ptr!=NULL)
    {
        if(!strcmp(ptr,"@"))
        return(NULL);
    strcpy(ptr,"@");
    free(ptr);
    }
    return(NULL);
}

void main()
{
char *txt,*test;

    txt=malloc(100);
    test=txt;
    strcpy(txt,"BUGS BUNNY PROGRAMS IN C AND LOVES UNIX");
    printf("txt=%s ... freeing txt....\n\r",txt);
    my_free(txt);
    printf("\n\rfreed ... txt=%s\n\r",txt);
	if ((int)txt == (int)'@')
		printf("freed memory is: txt = @");
	else if(*txt == '@')
		printf("freed memory is: *txt = @");
    my_free(test);
}

