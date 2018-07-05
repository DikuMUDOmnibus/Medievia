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
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <time.h>
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "spells.h"
#include "interp.h"

/*                  /External variables and functions\                    */
/*------------------------------------------------------------------------*/
extern void page_string(struct descriptor_data *d, char *str, int keep_internal);
extern int board(struct char_data *ch, int cmd, char *arg);
extern char menu[];


void fill_data(struct char_data *ch)
{
char buf[MAX_STRING_LENGTH];
char *p,c;
int line,x;
    if(!ch->desc)return;
    line=0;x=0;
    p=*ch->desc->str;
    while(p&&p[0]&&(p[0]==' '||p[0]=='\n'||p[0]=='\r'))p++;
    while(p&&p[0]){
	c=p[0];
	if(c!='\n'&&c!='\r'){
	    buf[x++]=c;
	    p++;
        }else{   
	    buf[x]=MED_NULL;
	    x=0;
	    strcpy(ch->desc->data[line],buf);
	    line++;
	    while(p[0]&&(p[0]=='\n'||p[0]=='\r'))
		p++;
	}
    }
    if(line==0&&x){
	 buf[x]=MED_NULL;
	 strcpy(ch->desc->data[line],buf);
    }
    ch->desc->curline=line;
}

int return_size(struct char_data *ch)
{
int x,size;
    if(!ch->desc)return(0);
    x=0;
    size=0;
    while(strlen(ch->desc->data[x])){
	size+=strlen(ch->desc->data[x++]);
	size+=2;/*for the /n/r */
    }
    return(size);
}
void show_menu(struct char_data *ch)
{
        strcpy(ch->p->queryprompt2,"Medievia Editor: E,S,J,L,I,W,R,T,P,C,D,A,H or M=Menu> ");
        ch->internal_use2=1000;
}
void send_menu(struct char_data *ch)
{
		sprintf(log_buf,"
\r          -=<(Medievia Editor)>=-
\r                 (E)dit
\r                 (S)ave
\r                (J)ustify
\r               Edit (L)ine
\r              (I)nsert line
\r             (W)ipe it clean
\r            Search & (R)eplace
\r           Look at (T)ext as is
\r          Swap text with (P)aper
\r            Spelling (C)hecker
\r               (D)elete line
\r                 (A)bort!
\r                  (H)elp
\r                  (M)enu
\r");
send_to_char(log_buf,ch);
}

void do_editor(struct char_data *ch, char *argument, int cmd)
{
    
char buf[MAX_INPUT_LENGTH], word[MAX_INPUT_LENGTH],*rest,*buf2,buffer[MAX_INPUT_LENGTH*2];
char pre[MAX_INPUT_LENGTH],suf[MAX_INPUT_LENGTH],*wordp,wordb[MAX_INPUT_LENGTH];
int prep,sufp,wordbp;
FILE *fl;
char notefile[250];
int x,y;
    if (IS_NPC(ch))
        return;
    if(!ch)return;
    if(!ch->desc)return;
    one_argument(argument, buf);
    x=0;
    if(cmd==9||cmd==0){
	SET_BIT(ch->specials.plr_flags, PLR_WRITING);
	if(!*ch->desc->str){
	    *ch->desc->str=str_dup(" ");
	    strcpy(*ch->desc->str,"");
	    send_to_char("**Text is Blank**\n\r",ch);
	}
	if(!strcmp(*ch->desc->str,"BlAnK")){
	    send_to_char("You start editing a fresh piece of paper...\n\r",ch);
	    strcpy(*ch->desc->str,"");
	}
        for(x=0;x<MAX_EDITOR_LINES;x++){
	    strcpy(ch->desc->data[x],"");
	}
	fill_data(ch);
	show_menu(ch);
	send_to_char(ch->p->queryprompt2,ch);
	if(strlen(ch->desc->editing)){
	    sprintf(log_buf,"$n starts editing [%s].",ch->desc->editing);
	    act(log_buf,TRUE,ch,0,0,TO_ROOM);
	}
	send_menu(ch);
        return;
     }
     if(cmd==1000){
	switch(buf[0]){
	    case 'M':
	    case 'm':
		send_menu(ch);
		return;
	    case 'E':
	    case 'e':
		send_to_char("\n\r\n\r-=<Ok start editing, Press ENTER on a blank line to return to Menu>=-\n\\n\r",ch);
		x=0;
		while(strlen(ch->desc->data[x])){
		    send_to_char("] ",ch);
		    send_to_char(ch->desc->data[x++],ch);
		    send_to_char("\n\r",ch);
		}
		ch->desc->curline=x;
		ch->internal_use2=1002;
                return;
	    case 'S':
	    case 's':
		x=return_size(ch);
		if(x>ch->desc->max_str){
	     	    sprintf(log_buf,"\n\rYOUR TEXT IS TO BIG, its %d characters, must be less than %d..\n\r",x,ch->desc->max_str);
		    send_to_char(log_buf,ch);
		    send_to_char("\n\r*********[PRESS ENTER TO CONTINUE]*********\n\r",ch);
		    ch->internal_use2=1001;
		    return;
        	}
		send_to_char("--[SAVING]--\n\r",ch);
		if(*ch->desc->str)
		    strcpy(*ch->desc->str,"");
		else
		    log_hd("###NO *ch->desc->str in SAVE do_edit!");
		if(!strlen(ch->desc->data[0])&&!strlen(ch->desc->data[1])){
		    strcpy(*ch->desc->str, "");
		}else{
		    x+=5;
	            if (!(*ch->desc->str=(char *)realloc(*ch->desc->str,x))){
	                perror("SAVING IN EDITOR1");
        	        exit(1);
            	    }
		    x=0;
		    if(ch->desc->oneline==2)
			{
			ch->specials.tmpstring = my_free(ch->specials.tmpstring);
			sprintf(notefile,"../notes/%s",GET_NAME(ch));
		        if (!(fl = med_open(notefile, "w")))
    			    {
		            perror ("##do_note:");
		            sprintf(log_buf,"Sorry, can't open the file %s.\n\r", notefile);
		            send_to_char(log_buf, ch);
			    REMOVE_BIT(ch->specials.plr_flags, PLR_WRITING);
			    return;
			    }
			open_files++;

		        while(strlen(ch->desc->data[x])){
    			    fputs(ch->desc->data[x++], fl);
			    fputs("\n",fl);
			}

		        med_close(fl);
			open_files--;
		        send_to_char("Message is saved in your note file.\n\r", ch);

		    }else{		    
		        while(strlen(ch->desc->data[x])){
		            strcat(*ch->desc->str, ch->desc->data[x++]);
			    if(!strlen(ch->desc->data[x])&&ch->desc->oneline==1)
			        break;
			    strcat(*ch->desc->str,"\n\r");
		        }
		    }
		}
       	        if(ch->desc->max_str==2123)
           	     board(ch, 1000, ""); 
	        if(ch->desc->editing)
		    sprintf(log_buf,"$n finishes editing a %s.",ch->desc->editing);
		else
		    sprintf(log_buf,"$n finishes editing.");
		if(ch->desc->connected==CON_PLAYING)
         	    act(log_buf,TRUE,ch,0,0,TO_ROOM);
	        if (ch->desc->connected == CON_EXDSCR)
        	{
	            write_to_q( menu, &ch->desc->output );
           	    ch->desc->connected = CON_SELECT_MENU;
		}
		ch->desc->oneline=0;
		ch->desc->max_str=0;
		ch->desc->str=NULL;
		ch->desc->curline=0;
		ch->internal_use2=0;
		ch->desc->editing[0]=MED_NULL;
		REMOVE_BIT(ch->specials.plr_flags, PLR_WRITING);
                return;
	    case 'J':
	    case 'j':
		prep=0;
		x=return_size(ch);
		if(x<5){
		    show_menu(ch);
		    return;
		}
		CREATE(buf2, char,(x+3)*2);
		y=0;
		strcpy(buf2,"");
		while(strlen(ch->desc->data[y])){
		    strcat(buf2,ch->desc->data[y]);
		    strcpy(ch->desc->data[y++],"");
		    strcat(buf2," ");
		}
		y=0;
		rest=&buf2[0];
		while(rest&&rest[0]){
		    rest=one_arg(rest,word);
		    prep++;
		    if((strlen(word)+strlen(ch->desc->data[y])+1)<80){
			strcat(ch->desc->data[y], " ");
			strcat(ch->desc->data[y],word);
		    }else{
			strcat(ch->desc->data[++y],word);
		    }
		}		
		sprintf(log_buf,"--[%d]--words processed.\n\r",prep);
		send_to_char(log_buf,ch);
		send_to_char("--==<<((*Justify Complete*))>>==--\n\r",ch);
		buf2 = my_free(buf2);
		show_menu(ch);
                return;
	    case 'L':
	    case 'l':
	    case 'I':
	    case 'i':
		y=0;
		while(strlen(ch->desc->data[y]))y++;
		y--;
		if(y<0){
		    send_to_char("There is NO TEXT yet!\n\r",ch);
	            show_menu(ch);
	            return;
		}
		x=0;
		if(ch->desc->curline>=MAX_EDITOR_LINES-1){
		    send_to_char("Your TEXT is at MAX LINES\n\r",ch);
	            show_menu(ch);
	            return;
		}

		while(strlen(ch->desc->data[x])){
		    sprintf(log_buf,"%2d] ",x);
		    send_to_char(log_buf,ch);
		    send_to_char(ch->desc->data[x++],ch);
		    send_to_char("\n\r",ch);
		}
		if(buf[0]=='L'||buf[0]=='l'){
		    strcpy(ch->p->queryprompt2,"\n\rEnter in line to edit or ENTER to abort> ");
		    ch->internal_use2=1003;
		}
		if(buf[0]=='I'||buf[0]=='i'){
		    strcpy(ch->p->queryprompt2,"\n\rType in line # to Insert new line BEFORE or ENTER to abort> ");
		    ch->internal_use2=1004;
		}
                return;
	    case 'W':
	    case 'w':
	        for(x=0;x<MAX_EDITOR_LINES;x++){
		    strcpy(ch->desc->data[x],"");
		}
		ch->desc->curline=0;
		send_to_char("-=<(**ALL DATA WIPED**)>=-\n\r",ch);
                return;
	    case 'R':
	    case 'r':
		strcpy(ch->p->queryprompt2,"\n\rType in the word to search and replace,
\rExample: willow Willow Tree.  willow will be replaced with Willow Tree.
\rSearch is sensitive, use exact capitalization> ");
                ch->internal_use2=1007;
		return;
                return;
	    case 'T':
	    case 't':
		if(ch->desc->editing)
		sprintf(log_buf,"\n\r\n\rThis is what the [%s] looks like so far...\n\r\n\r",ch->desc->editing);
		else
		sprintf(log_buf,"\n\r\n\rThis is what it looks like so far...\n\r\n\r");
		send_to_char(log_buf,ch);
		x=0;
		while(strlen(ch->desc->data[x])){
		    send_to_char("] ",ch);
		    send_to_char(ch->desc->data[x++],ch);
		    send_to_char("\n\r",ch);
		}
		send_to_char("\n\r*********[PRESS ENTER TO CONTINUE]*********\n\r",ch);
		ch->internal_use2=1001;
                return;
	    case 'P':
	    case 'p':
		send_to_char("This function is in development.\n\r",ch);
                return;
	    case 'C':
	    case 'c':
		send_to_char("This function is in development.\n\r",ch);
                return;
	    case 'D':
	    case 'd':
		y=0;
		while(strlen(ch->desc->data[y]))y++;
		y--;
		if(y<0){
		    send_to_char("There is NO TEXT yet!\n\r",ch);
	            show_menu(ch);
	            return;
		}
		x=0;
		while(strlen(ch->desc->data[x])){
		    sprintf(log_buf,"%2d] ",x);
		    send_to_char(log_buf,ch);
		    send_to_char(ch->desc->data[x++],ch);
		    send_to_char("\n\r",ch);
		}
		strcpy(ch->p->queryprompt2,"\n\rEnter in line to delete or ENTER to abort> ");
		ch->internal_use2=1006;
                return;
	    case 'A':
	    case 'a':
		ch->specials.tmpstring = my_free(ch->specials.tmpstring);
		ch->desc->oneline=0;
		ch->desc->max_str=0;
		ch->desc->str=NULL;
		ch->desc->curline=0;
		ch->internal_use2=0;
		ch->desc->editing[0]=MED_NULL;
		REMOVE_BIT(ch->specials.plr_flags, PLR_WRITING);
                return;
	    case 'H':
	    case 'h':
		send_to_char("This function is in development.\n\r",ch);
                return;
	    default:
	        return;
        }
    }
    if(cmd==1001){
	ch->internal_use2=1000;
	return;
    }
    if(cmd==1002){
	if(!argument||!argument[0]||argument[0]=='~'){
	    ch->internal_use2=1000;
	    return;
	}
	strcpy(ch->desc->data[ch->desc->curline++],argument);
	if(ch->desc->curline>=MAX_EDITOR_LINES-1){
	     send_to_char("Your TEXT is at MAX LINES\n\r",ch);
	     send_to_char("\n\r*********[PRESS ENTER TO CONTINUE]*********\n\r",ch);
	     ch->internal_use2=1001;

	}
	x=return_size(ch);
	if(x>ch->desc->max_str){
	     sprintf(log_buf,"\n\rYOUR TEXT IS TO BIG, its %d characters, must be less than %d..\n\r",x,ch->desc->max_str);
	     send_to_char(log_buf,ch);
	     send_to_char("\n\r*********[PRESS ENTER TO CONTINUE]*********\n\r",ch);
	     ch->internal_use2=1001;
        }
	return;
    }
    if(cmd==1003||cmd==1004){
	if(!buf[0]){
	    show_menu(ch);
	    return;
        }
	x=atoi(buf);
	y=0;
	while(strlen(ch->desc->data[y]))y++;
	y--;
	if(x<0||x>y){
	    sprintf(log_buf,"Valid lines are %d to %d.\n\r",x,y);
	    send_to_char(log_buf,ch);
	    return;
        }
	if(cmd==1004){/*inserting*/
	    strcpy(ch->desc->data[MAX_EDITOR_LINES-1],"");
	    for(y=MAX_EDITOR_LINES-1;y>x;y--){
		strcpy(ch->desc->data[y],"");
		if(strlen(ch->desc->data[y-1])){
		    strcpy(ch->desc->data[y],ch->desc->data[y-1]);
		}
	    }
 	}
	ch->desc->curline=x;
	ch->internal_use2=1005;
	if(cmd==1003)
	    sprintf(ch->p->queryprompt2,"-=<(OK enter in the text for the line)>=-\n\r\n\rOLD TEXT:\n\r> %s\n\r> ",ch->desc->data[x]);
	else
	    sprintf(ch->p->queryprompt2,"-=<(OK enter in the text for the line)>=-\n\r\n\r> ");
	return;
    }
    if(cmd==1005){
	if(!buf[0]){
	    strcpy(ch->desc->data[ch->desc->curline]," ");
	}else{
	    strcpy(ch->desc->data[ch->desc->curline],argument);;
	}
	show_menu(ch);
	return;
    }
    if(cmd==1006){
	if(!buf[0]){
	    show_menu(ch);
	    return;
        }
	x=atoi(buf);
	y=0;
	while(strlen(ch->desc->data[y]))y++;
	y--;
	if(x<0||x>y){
	    sprintf(log_buf,"Valid lines are %d to %d.\n\r",x,y);
	    send_to_char(log_buf,ch);
	    return;
        }
	for(y=x;y<MAX_EDITOR_LINES-1;y++)
		strcpy(ch->desc->data[y],ch->desc->data[y+1]);
	strcpy(ch->desc->data[MAX_EDITOR_LINES-1],"");
	send_to_char("\n\r-=<(*LINE DELETED*)>=-\n\r",ch);
	show_menu(ch);
	return;
    }
    if(cmd==1007){
        buf2=one_arg(argument,buf);
        if(!buf[0]||!buf2||!buf2[0]){ 
            show_menu(ch);
            return;
        }
	x=0;y=0;
	while(strlen(ch->desc->data[x])){
	    strcpy(buffer,ch->desc->data[x]);
	    strcpy(ch->desc->data[x],"");
	    do{
		rest=one_arg(buffer,word);
		if(!word[0]){
		    rest=NULL;
		    continue;
		}
		prep=0;sufp=0;wordp=&word[0];
		while(wordp[0]&&ispunct(wordp[0])){
		    pre[prep++]=wordp[0];
		    wordp++;
		}
		pre[prep]=MED_NULL;
		strcpy(wordb,wordp);
		wordbp=0;
		while(wordb[wordbp]&&!ispunct(wordb[wordbp]))
			wordbp++;
		wordb[wordbp]=MED_NULL;
		while(wordp[0]&&!ispunct(wordp[0]))
			wordp++;
                while(wordp[0]&&ispunct(wordp[0])){ 
                    suf[sufp++]=wordp[0];
                    wordp++;
                }
		suf[sufp]=MED_NULL;
		if(strlen(ch->desc->data[x]))
		    strcat(ch->desc->data[x]," ");
		if(pre[0])
		    strcat(ch->desc->data[x],pre);
		if(!str_cmp(wordb,buf)){
		    strcat(ch->desc->data[x],buf2);
		    y++;
		}else{
		    strcat(ch->desc->data[x],wordb);
		}
		if(suf[0])
		    strcat(ch->desc->data[x],suf);
		strcpy(buffer,rest);
	    }while(buffer[0]);  
	    x++;
	}
	sprintf(log_buf,"Done, [%d] Matches found and replaced.\n\r",y);
	send_to_char(log_buf,ch);
	show_menu(ch);
	return;
    }
}
