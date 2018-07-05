#include <stdio.h>

void main(void)
{
FILE *fp;
unsigned long int connect_count;

	printf("\n\rOK set the number for medievia's connections:> ");
    scanf("%ld",&connect_count);
	if((fp=fopen("connected.dat", "w"))!=NULL){
        fwrite(&connect_count,sizeof(unsigned long int), 1, fp);
        fclose(fp);
		printf("\n\rOk, done! Count set at %ld.\n\r",connect_count);
		return;
	}
	perror("Error opening file connected.dat:");                                          
}
