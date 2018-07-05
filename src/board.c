/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1995 INTENSE Software(tm) and Mike Krause	   *
*			   All rights reserved				   *
***************************************************************************/
/***************************************************************************
* This program belongs to INTENSE Software, and contains trade secrets of  *
* INTENSE Software.  The program and its contents are not to be disclosed  *
* to or used by any person who has not received prior authorization from   *
* INTENSE Software.  Any such disclosure or use may subject the violator   *
* to civil and criminal penalties by law.                                  *
***************************************************************************/


#include <stdlib.h>
#include <strings.h>
#include <stdlib.h> 
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
/*#include <limits.h>*/
#include <ctype.h> 
#include <fcntl.h>
#include <unistd.h> 
#include <sys/time.h>




#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "handler.h"

extern char global_color;
extern int file_to_string(char *name, char *buf);

int tmpboardsort[100];
char tmpboardname[100][80];
int boardsort[100];
char boardname[100][80];


void sort_board(void)
{
int x;
DIR *dp;
struct dirent *dirp=NULL; 
int file_number=-2,i=0; 
char done=0;
char found=0;
int sorted=0;
    for(x=0;x<100;x++){
	boardsort[x]=-1;
	boardname[x][0]=MED_NULL;
	tmpboardsort[x]=-1;
	tmpboardname[x][0]=MED_NULL;
    }
    if((dp=opendir("../board"))==NULL){
	log_hd("###Board, Cannot ]read directory");
	return;
    }
    while((dirp=readdir(dp))!=NULL){
	file_number++;
	if(strcmp(dirp->d_name,".")==0||strcmp(dirp->d_name,"..")==0)
	    continue;
    	strcpy(&tmpboardname[file_number-1][0],dirp->d_name);
	tmpboardsort[file_number-1]=file_number;

    }
    if(closedir(dp)<0)
	log_hd("###Cannot close Directory in BOARD");
    while(!done){
	found=0;
	for(x=0;x<100;x++)
	   if(tmpboardname[x][0]){
		found=1;
		i=x;
		break;
	   }		
	if(!found)return;
	for(x=0;x<100;x++)
	    if(tmpboardname[x][0])
	        if(strcmp(&tmpboardname[x][0],&tmpboardname[i][0])<0)
		    i=x;		
	strcpy(&boardname[sorted][0],&tmpboardname[i][0]);
	boardsort[sorted]=tmpboardsort[i];
	tmpboardname[i][0]=MED_NULL;
	sorted++;

    }
}


int board(struct char_data *ch, int cmd, char *arg)
{
char message[MAX_STRING_LENGTH];
FILE *fh;
int file_number=-2,i; 
char display[256]; 
char buf[MAX_INPUT_LENGTH]; 
char subject[36];
struct obj_data *paper=NULL; 
char filename[160]; 
struct tm *thetime=NULL;
long t; 
char stra[100],strb[100];

    if (!ch)return(TRUE);
    if (!ch->desc)
	return(FALSE); /* By MS or all NPC's will be trapped at the board */
    
    switch (cmd) {
	case 13: /* save and quit */
	    send_to_char("Saving and Quiting is not allowed in the Library\n\rPlease refrain from any activity in this room so you don't disturb\n\r the people using the Message Board.\n\r", ch);
	    return(1);	
	case 15:  /* look */
	    one_argument(arg,buf);
	    if(strcmp("board",buf))
		return(FALSE);
	    global_color=1;
	    act("$n glances at the bulletin board.",FALSE,ch,0,0,TO_ROOM);
 	    ch->specials.setup_page=1;
 	    page_setup[0]=MED_NULL;
	    send_to_char("\n\r                      MEDIEVIA BULLETIN BOARD\n\r",ch);
	    global_color=33;
	    send_to_char("   <commands: (look board)(read #)(put paper board)(remove #)\n\r",ch);
	    global_color=1;
	    send_to_char("\n\r[#]-[  DATE  ][         AUTHOR          ]    SUBJECT\n\r\n\r",ch);
	    i=1;
	    global_color=35;
	    while(boardname[i-1][0]){
	       sprintf(display,"[%2d]-%s\n\r",i,&boardname[i-1][0]);
       	       if((!isname("GOD",display))||(GET_LEVEL(ch)>31))
                  send_to_char(display,ch); 
	       i++;
	    }
	    global_color=0;
 	    ch->specials.setup_page=0;  
    	    page_string(ch->desc, page_setup, 1);
	    return(TRUE);
	    break;
	case 223: /* put */
	    one_argument(arg,buf);

	    if(!isname("board",arg)){
		return(FALSE); 
	    }
	    if(GET_LEVEL(ch)<6){
		send_to_char("Sorry, do to the abuse of posting, you must be level 6+ to post.\n\r",ch);	
		return(TRUE);
	    }
	    if (!(paper = get_obj_in_list_vis(ch, buf, ch->carrying))){
	        sprintf(log_buf, "You have no %s.\n\r", buf);
	        send_to_char(log_buf, ch);
	        return(TRUE);
	    }
	    if(strlen(paper->action_description)<3){
		send_to_char("There is nothing writen on it!\n\r",ch);
		return(TRUE);
	    }
	    if(paper->obj_flags.type_flag != ITEM_NOTE){
	        send_to_char("That thing is not a paper!\n\r",ch);
	        return(TRUE);
 	    }
	    act("$n tacks a note on the bulletin board.",FALSE,ch,0,0,TO_ROOM);
	    send_to_char("OK, you tack the paper on the board.\n\r",ch);
	    t = time(0); 
	    thetime=localtime(&t); 
	    subject[35]=MED_NULL;
	    for(i=0;i<35&&paper->action_description[i];i++){
		subject[i]=paper->action_description[i];;
		if(subject[i]=='\n'||subject[i]=='\r'){
		    subject[i]=MED_NULL;
		    i=35;
		}
	    }
	    if(strlen(subject)<2){
		send_to_char("Sorry, Your first line on your paper(which is the subject), must be longer...\n\r",ch);
		return(TRUE);
	    }
	    i=0;
	    while(subject[i])
		if(subject[i++]=='/'){
		    send_to_char("Sorry a '/' is not a legal character!\n\r",ch);
		    return(TRUE);
		}
	    sprintf(filename,"../board/[%2d-%2d-%2d][%24s ]%s",
		thetime->tm_year,
		thetime->tm_mon+1,
		thetime->tm_mday,
		GET_NAME(ch),
		subject
	    );
	    if((fh=med_open(filename, "w"))!=NULL){
		open_files++;
		if(fputs(&paper->action_description[0],fh)<1){
		    log_hd("###Error WRITING paper on board");
		    send_to_char("Sorry, disk error.\n\r",ch);
		    med_close(fh);
		    open_files--;
		    return(TRUE);
		}
		med_close(fh);
		open_files--;
		obj_from_char(paper);
		extract_obj(paper);
		sort_board();
		return(TRUE);
	    }else{
		log_hd("###Could not open board file for writing.");
		send_to_char("Sorry disk error.\n\r",ch);
		return(TRUE);
	    }
	    return(TRUE);
	    break;
	case 63: /* read */
	    one_argument(arg,buf);
	    if(!*buf||!isdigit(*buf))
		return(FALSE);
	    if(!(file_number=atoi(buf)))
		return(FALSE);
	    strcpy(filename,&boardname[file_number-1][0]);
	    if(!filename[0]){
		send_to_char("You realize that message does not exist.\n\r",ch);
		return(TRUE);
	    }		
	    strcpy(buf,"../board/");
	    strcat(buf,filename);
	    file_to_string(buf, message);
	    sprintf(log_buf,"%s\n\r",filename);
	    if((isname("GOD",filename))&&(GET_LEVEL(ch)<=31)){
		send_to_char("Sorry, that is a GOD only message.\n\r",ch);
		return(TRUE);
	    }
	    send_to_char("\n\r                      MEDIEVIA BULLETIN BOARD\n\r",ch);
	    send_to_char("\n\r[  DATE  ][         AUTHOR          ]    SUBJECT\n\r\n\r",ch);
	    send_to_char(log_buf,ch);
	    page_string(ch->desc, message, 1);	
	    return(TRUE);
	    break;
	case 66: /* remove */
	    one_argument(arg,buf);
	    if(!*buf||!isdigit(*buf))
		return(FALSE);
	    if(!(file_number=atoi(buf)))
		return(FALSE);
	    strcpy(filename,&boardname[file_number-1][0]);
	    if(!filename[0]){
		send_to_char("You realize that message does not exist.\n\r",ch);
		return(TRUE);
	    }		
	    for(i=11;i<=35;i++)
		stra[i-11]=filename[i];
	    stra[i-11+1]=MED_NULL;
	    one_argument(stra,strb);
	    if(GET_LEVEL(ch)<35){
		send_to_char("Only GODS can remove messages at the moment!\n\r",ch);
		return(TRUE);
	    }
	    strcpy(buf,"../board/");
	    strcat(buf,filename);
	    unlink(buf);
	    send_to_char("You take the paper off the board and throw it out.\n\r",ch);
	    sort_board();
	    return(TRUE);
	    break;
	default:
	    return(FALSE);
	    break;
    } 
    return(FALSE);
}

