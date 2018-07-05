#include <stdio.h>
#include <malloc.h>
#include <string.h>


main()
{
char *test;

    test=malloc(100);
    strcpy(test,"THIS IS A TEST!");
    printf("test=%s before free\n",test);
    free(test);
    printf("test=%s after free\n",test);
}
