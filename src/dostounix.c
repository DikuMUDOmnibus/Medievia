#include <stdio.h>
#include <stdlib.h>

void main(void)
{
FILE *in, *out;
int c;
char buf[100];
    printf("\n\rConvert Medievias WORLD file? (y/n)> ");
    gets(buf);
    if(buf[0]=='y'){
        if((in=fopen("medievia.wld", "rb"))==NULL){
    	    printf("Error opening file\n\r");
            return;
        }
        printf("file Medievia.wld opened for input\n\r");
        if((out=fopen("converted.file", "w"))==NULL){
            printf("Error opening output file\n\r");
	    return;
        }
        printf("file converted.file opened for output\n\r");
        while((c=getc(in))!=EOF)
	    if(c!=13)
	        putc(c, out);
        printf("File converted\n\r");
	fclose(in);
	fclose(out);
	remove("medievia.wld");
	rename("converted.file","medievia.wld");
    }
    printf("\n\rConvert Medievias ZONE file? (y/n)> ");
    gets(buf);
    if(buf[0]=='y'){
        if((in=fopen("medievia.zon", "rb"))==NULL){
    	    printf("Error opening file\n\r");
            return;
        }
        printf("file Medievia.zon opened for input\n\r");
        if((out=fopen("converted.file", "w"))==NULL){
            printf("Error opening output file\n\r");
	    return;
        }
        printf("file converted.file opened for output\n\r");
        while((c=getc(in))!=EOF)
	    if(c!=13)
	        putc(c, out);
        printf("File converted\n\r");
	fclose(in);
	fclose(out);
	remove("medievia.zon");
	rename("converted.file","medievia.zon");
    }
    printf("\n\rConvert Medievias OBJECT file? (y/n)> ");
    gets(buf);
    if(buf[0]=='y'){
        if((in=fopen("medievia.obj", "rb"))==NULL){
    	    printf("Error opening file\n\r");
            return;
        }
        printf("file Medievia.obj opened for input\n\r");
        if((out=fopen("converted.file", "w"))==NULL){
            printf("Error opening output file\n\r");
	    return;
        }
        printf("file converted.file opened for output\n\r");
        while((c=getc(in))!=EOF)
	    if(c!=13)
	        putc(c, out);
        printf("File converted\n\r");
	fclose(in);
	fclose(out);
	remove("medievia.obj");
	rename("converted.file","medievia.obj");
    }
    printf("\n\rConvert Medievias MOBILE file? (y/n)> ");
    gets(buf);
    if(buf[0]=='y'){
        if((in=fopen("medievia.mob", "rb"))==NULL){
    	    printf("Error opening file\n\r");
            return;
        }
        printf("file Medievia.mob opened for input\n\r");
        if((out=fopen("converted.file", "w"))==NULL){
            printf("Error opening output file\n\r");
	    return;
        }
        printf("file converted.file opened for output\n\r");
        while((c=getc(in))!=EOF)
	    if(c!=13)
	        putc(c, out);
        printf("File converted\n\r");
	fclose(in);
	fclose(out);
	remove("medievia.mob");
	rename("converted.file","medievia.mob");
    }
    printf("\n\rConvert Medievias SHOPS file? (y/n)> ");
    gets(buf);
    if(buf[0]=='y'){
        if((in=fopen("medievia.shp", "rb"))==NULL){
    	    printf("Error opening file\n\r");
            return;
        }
        printf("file Medievia.shp opened for input\n\r");
        if((out=fopen("converted.file", "w"))==NULL){
            printf("Error opening output file\n\r");
	    return;
        }
        printf("file converted.file opened for output\n\r");
        while((c=getc(in))!=EOF)
	    if(c!=13)
	        putc(c, out);
        printf("File converted\n\r");
	fclose(in);
	fclose(out);
	remove("medievia.shp");
	rename("converted.file","medievia.shp");
    }
    printf("\n\rConvert Medievias SOCIALS.TXT file? (y/n)> ");
    gets(buf);
    if(buf[0]=='y'){
        if((in=fopen("social.txt", "rb"))==NULL){
    	    printf("Error opening file\n\r");
            return;
        }
        printf("file social.txt opened for input\n\r");
        if((out=fopen("converted.file", "w"))==NULL){
            printf("Error opening output file\n\r");
	    return;
        }
        printf("file converted.file opened for output\n\r");
        while((c=getc(in))!=EOF)
	    if(c!=13)
	        putc(c, out);
        printf("File converted\n\r");
	fclose(in);
	fclose(out);
	remove("social.txt");
	rename("converted.file","social.txt");
    }
}
