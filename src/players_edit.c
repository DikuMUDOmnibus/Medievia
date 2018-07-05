#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

int deleted,total;

FILE *lfd;

void purge_date(char *name)
{
struct stat statbuf;
struct tm *tma;

	if(stat(name,&statbuf)==-1){
	    printf("(Stat Error)");
	    return;
	}
	total++;
	tma=gmtime(&statbuf.st_mtime);

	if(tma->tm_year>=95&&tma->tm_mon>=3)
	    printf("K");/*keep*/
	else{
	    deleted++;
	    printf("d");/*delete*/	
	    unlink(name);
	    strcat(name,".*");
	    unlink(name);
	    strcat(name,"\n");
	    fputs(name,lfd);
	}
}


main()
{
DIR *dp;
struct dirent *dirp;
char filename[250];
char fullname[250];
unsigned char l;

	total=0;
	deleted=0;	
	printf("\n\r\n\rSTARTING.....\n\r");
        lfd=fopen("purged_people", "a");

    for(l='a';l<='z';l++){
	sprintf(filename,"../save/%c",l);
	if((dp=opendir(filename))!=NULL){
	    printf("\n\rDirectory [%s] Opened for Reading\n\rProcessing\n\r",filename);
	    while((dirp=readdir(dp))!=NULL){
		if(!strcmp(dirp->d_name,".")||!strcmp(dirp->d_name,".."))
		    continue;
		sprintf(fullname,"%s/%s",filename,dirp->d_name);
		purge_date(fullname);
	    }
	    closedir(dp);
	}else{
	    printf("ERROR OPENING %s  DIRECTORY",filename);
	}
    }
    fclose(lfd);
	printf("\n\rDONE.  (%d)total (%d)deleted (%d) kept\n\r",total,deleted,total-deleted);
}
