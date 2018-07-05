#include <stdio.h>
#include <string.h>
#include <malloc.h>


void *my_free(void *ptr)
{
    if(!ptr){
	printf("Like ptr is NULL master");
	exit(0);
    }
    printf("IN FREE ptr=(%s)\n\r",ptr);
    if(!strcmp(ptr,"@")){
	printf("@ FOUND BABY! \n\r");
	exit(0);
    }
    strcpy(ptr,"@");
    free(ptr);
    return(NULL);
}

main()
{
char *txt,*test;

    txt=malloc(100);
    test=txt;
    strcpy(txt,"BUGS BUNNY PROGRAMS IN C AND LOVES UNIX");
    printf("txt=%s\n\rfreeing txt....\n\r",txt);
    my_free(txt);
    printf("\n\rtxt=%s\n\r",txt);
    my_free(test);
}
