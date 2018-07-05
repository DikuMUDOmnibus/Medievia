
#include <stdio.h>

main()
{
int i;
char buf[100];
    for(i=30;i<=40;i++) {
	 if(i == 12) continue;
	 sprintf(buf,"\033[0m\033[%dm[-=%2d=-]\033[1m\033[%dm[-=%2d=-]",i,(i+100),i,i);
	 printf(buf);
	 if( (i-29) % 4 == 0)
	    printf("\r\n");
     }

	printf("\r\n");
}
