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
extern int port;
extern struct room_data *world[MAX_ROOM];  
extern struct char_data *mobs[MAX_MOB];  
extern struct obj_data *objs[MAX_OBJ];  
extern int top_of_world;  
extern struct obj_data *object_list;  
extern struct char_data *character_list;  
extern struct zone_data zone_table_array[MAX_ZONE];  
extern struct zone_data *zone_table;  
extern int top_of_zone_table;  
extern struct index_data mob_index_array[MAX_MOB]; 
extern struct index_data *mob_index;  
extern struct index_data obj_index_array[MAX_OBJ];  
extern struct index_data *obj_index;  
extern int top_of_mobt;  
extern int top_of_objt; 
extern struct descriptor_data *descriptor_list;  
extern int dice(int number, int size);  
extern int number(int from, int to); 
extern void space_to_underline(char *text);
extern void page_string(struct descriptor_data *d, char *str, int keep_internal); 
extern int number_of_rooms;
extern char free_error[100];
extern char global_color;



struct char_data * pick_victim(struct char_data *ch);

bool is_formed(struct char_data *ch)
{
int x,y,n;

    n=0;
    if(ch->master==ch){
        for(x=0;x<3;x++){
	    for(y=0;y<3;y++){
    	        if(ch->formation[x][y])
		    n++;
		if(n>1){
		    return TRUE;
		}
            }
        }
    }else{
	return TRUE;
    }
    return FALSE;
}

bool IS_IN_FRONT(struct char_data *ch)
{
int col,row,room;

    if(!is_formed(ch))return(TRUE);
    room=ch->in_room;
    for(row=0;row<3;row++)
	for(col=0;col<3;col++)
	    if(ch->master->formation[row][col]==ch){
		if(ch->master->formation[0][col]){
		    if(ch->master->formation[0][col]==ch)return(TRUE);
		    else{
			if(ch->master->formation[0][col]->in_room==room)
			    return(FALSE);
		    }
		}
		if((ch->master->formation[0][0]&&ch->master->formation[0][0]->in_room==room)||
		   (ch->master->formation[0][1]&&ch->master->formation[0][1]->in_room==room)||
		   (ch->master->formation[0][2]&&ch->master->formation[0][2]->in_room==room))return(FALSE);
		if(ch->master->formation[1][col]){
		    if(ch->master->formation[1][col]==ch)return(TRUE);
		    else{
			if(ch->master->formation[1][col]->in_room==room)
			    return(FALSE);
		    }
		}
		if((ch->master->formation[1][0]&&ch->master->formation[1][0]->in_room==room)||
		   (ch->master->formation[1][1]&&ch->master->formation[1][1]->in_room==room)||
		   (ch->master->formation[1][2]&&ch->master->formation[1][2]->in_room==room))return(FALSE);
		return(TRUE);
	    }
    return(FALSE);		

}

bool IS_IN_BACK(struct char_data *ch)
{
int col,row,room;

    if(!is_formed(ch))return(TRUE);
    room=ch->in_room;
    for(row=0;row<3;row++)
	for(col=0;col<3;col++)
	    if(ch->master->formation[row][col]==ch){
		if(ch->master->formation[2][col]){
		    if(ch->master->formation[2][col]==ch)return(TRUE);
		    else{
			if(ch->master->formation[2][col]->in_room==room)
			    return(FALSE);
		    }
		}
		if((ch->master->formation[2][0]&&ch->master->formation[2][0]->in_room==room)||
		   (ch->master->formation[2][1]&&ch->master->formation[2][1]->in_room==room)||
		   (ch->master->formation[2][2]&&ch->master->formation[2][2]->in_room==room))
			return(FALSE);
		if(ch->master->formation[1][col]){
		    if(ch->master->formation[1][col]==ch)return(TRUE);
		    else{
			if(ch->master->formation[1][col]->in_room==room)
			    return(FALSE);
		    }
		}
		if((ch->master->formation[1][0]&&ch->master->formation[1][0]->in_room==room)||
		   (ch->master->formation[1][1]&&ch->master->formation[1][1]->in_room==room)||
		   (ch->master->formation[1][2]&&ch->master->formation[1][2]->in_room==room))return(FALSE);
		return(TRUE);
	    }
    return(FALSE);		

}


int return_perc(int amount, int times)
{
float a,t,p;
int r;

    a=amount;
    t=times;
    p=a/t;
    p*=100;
    r=(int)p;
    return(r);
}


void do_formtest(struct char_data *ch, char *argument, int cmd)
{
int array[3][3];
int x,y,n,f;
char buf[MAX_INPUT_LENGTH];
struct char_data *m=NULL,*v=NULL;

    if(!is_formed(ch)){
	send_to_char("You have no formation.\n\r",ch);
	return;
    }
    m=ch->master;
    if(!m)return;
    one_argument(argument,buf);
    if(!buf||!buf[0]){
	n=150;
    }else{
	n=atoi(buf);
	if(n<1||n>5000){
	    send_to_char("Number must be from 1 to 5000.\n\r",ch);
	    return;
	}
    }
    for(x=0;x<3;x++)
	for(y=0;y<3;y++)
	    array[x][y]=0;

    for(f=0;f<n;f++){
	v=pick_victim(m);
	for(x=0;x<3;x++)
	    for(y=0;y<3;y++)
		if(m->formation[x][y]==v){
		    array[x][y]++;;
		}


    }
    sprintf(log_buf," Breakdown of firing hits and %%'s
\r~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r");
	send_to_char(log_buf,ch);
	    if(!array[0][0])
		sprintf(log_buf,"---- --%%     ");
	    else
		sprintf(log_buf,"%4d %2d%%     ",array[0][0],return_perc(array[0][0],n));
	    send_to_char(log_buf,ch);
	    if(!array[0][1])
		sprintf(log_buf,"---- --%%     ");
	    else
		sprintf(log_buf,"%4d %2d%%     ",array[0][1],return_perc(array[0][1],n));
	    send_to_char(log_buf,ch);
	    if(!array[0][2])
		sprintf(log_buf,"---- --%%");
	    else
		sprintf(log_buf,"%4d %2d%%     ",array[0][2],return_perc(array[0][2],n));
	    send_to_char(log_buf,ch);
	    send_to_char("\n\r",ch);
	    if(!array[1][0])
		sprintf(log_buf,"---- --%%     ");
	    else
		sprintf(log_buf,"%4d %2d%%     ",array[1][0],return_perc(array[1][0],n));
	    send_to_char(log_buf,ch);
	    if(!array[1][1])
		sprintf(log_buf,"---- --%%     ");
	    else
		sprintf(log_buf,"%4d %2d%%     ",array[1][1],return_perc(array[1][1],n));
	    send_to_char(log_buf,ch);
	    if(!array[1][2])
		sprintf(log_buf,"---- --%%");
	    else
		sprintf(log_buf,"%4d %2d%%     ",array[1][2],return_perc(array[1][2],n));
	    send_to_char(log_buf,ch);
	    send_to_char("\n\r",ch);
	    if(!array[2][0])
		sprintf(log_buf,"---- --%%     ");
	    else
		sprintf(log_buf,"%4d %2d%%     ",array[2][0],return_perc(array[2][0],n));
	    send_to_char(log_buf,ch);
	    if(!array[2][1])
		sprintf(log_buf,"---- --%%     ");
	    else
		sprintf(log_buf,"%4d %2d%%     ",array[2][1],return_perc(array[2][1],n));
	    send_to_char(log_buf,ch);
	    if(!array[2][2])
		sprintf(log_buf,"---- --%%");
	    else
		sprintf(log_buf,"%4d %2d%%     ",array[2][2],return_perc(array[2][2],n));
	    send_to_char(log_buf,ch);
	    send_to_char("\n\r",ch);
}



struct char_data *pick_victim(struct char_data *ch)
{
int room,deep,row,col,num=50;
char mobflag;
    if(!is_formed(ch))return(ch);
    room=ch->in_room;
    if(room<0){
	log_hd("##ch->in_room <0 in pick_victim");
	return(NULL);
    }
    deep=1;
    ch=ch->master;
    if(IS_NPC(ch))mobflag=1;
    else mobflag=0;
    while(1){
	if(deep>500){
	    log_hd("## went 500 deep and gave up in pick_victim");
	    return NULL;
	}
	deep++;
	col=number(0,2);
	if(!ch->formation[0][col]
	   &&!ch->formation[1][col]
	   &&!ch->formation[2][col]
	)continue;
	/* ok we have a good column */
	row=0;
	if(ch->formation[row][col]){
	    if(mobflag&&IS_NPC(ch->formation[row][col]))num=30;
	    else num=70;
	    if(number(0,100)<num)
		if(room==ch->formation[row][col]->in_room)
		    return(ch->formation[row][col]);
	}
	if(!ch->formation[row][col])
	    num=50;
	    if(number(0,100)<num)
		continue;
	row=1;
	if(ch->formation[row][col]){
	    if(mobflag&&IS_NPC(ch->formation[row][col]))num=30;
	    else num=70;
	    if(number(0,100)<num)
		if(room==ch->formation[row][col]->in_room)
		    return(ch->formation[row][col]);
	}
	if(!ch->formation[row][col])
	    num=50;
	    if(number(0,100)<num)
		continue;
	row=2;
	if(ch->formation[row][col]){
	    if(mobflag&&IS_NPC(ch->formation[row][col]))num=30;
	    else num=70;
	    if(number(0,100)<num)
		if(room==ch->formation[row][col]->in_room)
		    return(ch->formation[row][col]);
	}
    }
}


bool put_in_formation(struct char_data *leader, struct char_data *follower)
{
int x,y;

    if(!leader||!follower||leader==follower)
	return FALSE;
    for(x=0;x<3;x++){
	for(y=0;y<3;y++){
	    if(!leader->formation[x][y]){
		leader->formation[x][y]=follower;
		follower->master=leader;
		for(x=0;x<3;x++)
		    for(y=0;y<3;y++)
			if(follower->formation[x][y]==follower)
			    follower->formation[x][y]=0;
		return TRUE;
	    }
	}
    }
    return FALSE;
}

void remove_from_formation(struct char_data *ch)
{
int x,y;
struct char_data *master=NULL;

    if(ch->master==ch)return;
    master=ch->master;
    if(!master)return;
    for(x=0;x<3;x++){
	for(y=0;y<3;y++){
	    if(ch->master->formation[x][y]==ch){
		sprintf(log_buf,"%s is removed from %s's formation.\n\r",
			(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
			(IS_NPC(ch->master) ? 
				ch->master->player.short_descr : GET_NAME(ch->master))
			);
		ch->master->formation[x][y]=NULL;
		for(x=0;x<3;x++)
		    for(y=0;y<3;y++)
			if(ch->master->formation[x][y]&&ch->master->formation[x][y]->in_room!=ch->in_room)
			    send_to_char(log_buf,ch->master->formation[x][y]);
		send_to_room(log_buf,ch->in_room);
		ch->master=ch;
		ch->formation[0][1]=ch;
		return;
	    }
	}
    }
    if(!is_formed(master)){
        for(x=0;x<3;x++)
	    for(y=0;y<3;y++)
		master->formation[x][y]=NULL;
	master->formation[0][1]=master;
    }
}

void die_formation(struct char_data *ch)
{
int x,y;

    if(ch->master!=ch){
	remove_from_formation(ch);
	return;
    }
    for(x=0;x<3;x++){
	for(y=0;y<3;y++){
	    if(ch->formation[x][y]){
	        if(ch->formation[x][y]==ch){
		    ch->formation[x][y]=NULL;
	   	}else{
		    remove_from_formation(ch->formation[x][y]); 
		}
	    }
	}
    }
    ch->formation[0][1]=ch;    
}

void do_reform(struct char_data *ch, char *argument, int cmd)
{
int ox,oy,x,y;
char name[MAX_INPUT_LENGTH];
char pos[MAX_INPUT_LENGTH];
char r,c;
char or[10],oc[10],nr[10],nc[10];
struct char_data *player=NULL;


    ox=0;oy=0;x=0;y=0;
    half_chop(argument, name, pos);
    if (!name[0]||!pos[0]) {
        sprintf(log_buf,"   SYNTAX: reform name position(row,column)
\rPositions: fl fc fr
\r           cl cc cr
\r           bl bc br
\rRows    f=front c=center b=back
\rColumns l=left  c=center r=right
\r
\rExample: reform vryce bl    (put Vryce in back left position)\n\r");
		send_to_char(log_buf,ch);
        return; 
    }
    if (!(player = get_char(name))) {
        send_to_char("I see no person by that name here!\n\r", ch);
        return;
    }   
    if(ch->master!=ch){
	send_to_char("You must be leading to use reform.\n\r",ch);		
	return;
    }
    if(!is_formed(ch)){
	send_to_char("You're alone in your group, stay put.\n\r",ch);
	return;
    }
    ox=10;
    for(x=0;x<3;x++)
	for(y=0;y<3;y++)
	    if(ch->formation[x][y]==player){
		ox=x;oy=y;
	    }
    if(ox>2){
	send_to_char("That person is not in your formation.\n\r",ch);
	return;
    }

    r=LOWER(pos[0]);c=LOWER(pos[1]);
    if(r!='f'&&r!='c'&&r!='b'){
	send_to_char("Rows are f,c and b.\n\r",ch);
	return;
    }
    if(c!='l'&&c!='c'&&c!='r'){
	send_to_char("Columns are l,c and r.\n\r",ch);
	return;
    }
    if(r=='f')x=0;
    if(r=='c')x=1;
    if(r=='b')x=2;
    if(c=='l')y=0;
    if(c=='c')y=1;
    if(c=='r')y=2;
    if(ch->formation[x][y]){
	sprintf(log_buf,"Someone is in position %c%c already.\n\r",r,c);
	send_to_char(log_buf,ch);
	return;
    }
    ch->formation[x][y]=player;
    ch->formation[ox][oy]=NULL;
    if(ox==0)strcpy(or,"Front");
    if(ox==1)strcpy(or,"Center");
    if(ox==2)strcpy(or,"Back");
    if(oy==0)strcpy(oc,"Left");
    if(oy==1)strcpy(oc,"Center");
    if(oy==2)strcpy(oc,"Right");
    if( x==0)strcpy(nr,"Front");
    if( x==1)strcpy(nr,"Center");
    if( x==2)strcpy(nr,"Back");
    if( y==0)strcpy(nc,"Left");
    if( y==1)strcpy(nc,"Center");
    if( y==2)strcpy(nc,"Right");
    sprintf(log_buf,"%s is moved from %s/%s to %s/%s.\n\r",
		(IS_NPC(player) ? player->player.short_descr : GET_NAME(player)),
		or,oc,nr,nc);
    for(x=0;x<3;x++)
	for(y=0;y<3;y++)
	    if(ch->formation[x][y]&&ch->formation[x][y]->in_room!=ch->in_room)
		send_to_char(log_buf,ch->formation[x][y]);
    send_to_room(log_buf,ch->in_room);
}

void do_unform(struct char_data *ch, char *argument, int cmd)
{
int x,y;
char name[MAX_INPUT_LENGTH];
struct char_data *player=NULL;

    one_argument(argument, name);
    if (*name) {
        if (!(player = get_char(name))) {
            send_to_char("I see no person by that name here!\n\r", ch);
            return;
        }   
    } else {
        send_to_char("SYNTAX: unform name (remove him/her from your formation)\n\r        unform YOURNAME unform your whole group.\n\r", ch);
        return; 
    }
    if(ch->master!=ch){
	send_to_char("You must be leading to use unform.\n\r",ch);		
	return;
    }
    if(player==ch){
	if(!is_formed(ch)){
	    send_to_char("But you are not formed!\n\r",ch);
	    return;
	}
	die_formation(ch);
    }
    for(x=0;x<3;x++){
	for(y=0;y<3;y++){
	    if(ch->formation[x][y]){
		if(ch->formation[x][y]==player){
		    remove_from_formation(player);
		    return;
		}
	    }
	}
    }
    send_to_char("That player is not in your formation.\n\r",ch);
}

void do_follow(struct char_data *ch, char *argument, int cmd)
{
char name[MAX_INPUT_LENGTH];
struct char_data *leader=NULL;


    if(IS_NPC(ch))return;
    one_argument(argument, name);
                 
    if(cmd==1000){
	if(ch->ask_for_follow){
	    if(!ch->ask_for_follow->desc){
		send_to_char("That person seems to have lost link.\r\n", ch);
		ch->ask_for_follow=NULL;
		ch->p->querycommand=0;
		return;
	    }
	    if(name[0]=='y'||name[0]=='Y'){
		if(ch->ask_for_follow->master!=ch->ask_for_follow){
		    send_to_char("Too late, that person has joined another.\n\r",ch);
		    ch->ask_for_follow=NULL;
		    ch->p->querycommand=0;
		    return;
		}
		if(!put_in_formation(ch,ch->ask_for_follow)){
		    send_to_char("Formation is FULL!",ch);
		    send_to_char("Formation is FULL!",ch->ask_for_follow);
		}else{
		    act("OK! $n allows you, you start following.",TRUE,ch,0,ch->ask_for_follow,TO_VICT);
		    act("Ok, $N starts following you.",TRUE,ch,0,ch->ask_for_follow,TO_CHAR);
		    act("$N starts following $n.",TRUE,ch,0,ch->ask_for_follow,TO_NOTVICT);
		}
	    }else{
	        sprintf(log_buf,"Sorry, %s won't allow you to follow!\n\r",GET_NAME(ch));
	        send_to_char(log_buf,ch->ask_for_follow);
	        act("Ok, $N can't follow now.",TRUE,ch,0,ch->ask_for_follow,TO_CHAR);
	    }
	}
	ch->ask_for_follow=NULL;
	ch->p->querycommand=0;
	return;
    }
    if (*name) {
        if (!(leader = get_char_room_vis(ch, name))) {
            send_to_char("I see no person by that name here!\n\r", ch);
            return;
        }   
    } else {
        send_to_char("Who do you wish to follow?\n\r", ch);
        return; 
    }
    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master!=ch)) {
        act("But you only feel like following $N!",
           FALSE, ch, 0, ch->master, TO_CHAR);  
	return;
    }
    if (leader == ch) {
         if (ch->master==ch&&!is_formed(ch)) {
             send_to_char("You are already following yourself.\n\r",ch);
             return;
         }
         if (ch->master==ch&&is_formed(ch)) {
	     send_to_char("You are following yourself. If you want to remove everyone from \n\ryour formation, type unform YOURNAME\n\r",ch);
	     return;
	 }
	 remove_from_formation(ch);
         return;
    }
    if(ch->master!=ch){
	send_to_char("You must follow yourself before following someone else.\n\r",ch);
	return;
    }
    if(leader->specials.fighting){
	send_to_char("That person is fighting!\n\r",ch);
	return;
    }
    if(IS_UNDEAD(leader)){
 	send_to_char("Sorry that person appears to be dead.\n\r",ch);
	return;
    }
    if(!leader->desc||leader->desc->connected!=CON_PLAYING){
	send_to_char("That person has lost link.\n\r",ch);
	return;
    }
    if(!AWAKE(leader)){
	send_to_char("That person is not awake...\n\r",ch);
	return;
    }
    if(leader->p->querycommand){
	send_to_char("Ask in a moment...\n\r",ch);
	return;
    }
    if(leader->master!=leader){
	send_to_char("That person is not a leader(following someone else).\n\r",ch);
	return;
    }
	if(is_formed(ch)){
		send_to_char("You are already formed.\n\r", ch);
		return;
	}
    if(leader->desc->connected!=CON_PLAYING)
	return;/* just in case I missed something */
    act("You ask $N if you can follow $M.",TRUE,ch,0,leader,TO_CHAR);   
    act("$n asks $N if $e can follow $M.",TRUE,ch,0,leader,TO_NOTVICT);
    act("$n asks if $e can follow you.",TRUE,ch,0,leader,TO_VICT);
    leader->ask_for_follow=ch;
    sprintf(leader->p->queryprompt,"Can %s follow you? (y/n)> ",GET_NAME(ch));
    leader->p->querycommand=1000;
    leader->p->queryfunc=do_follow;
}

void center_in_logbuf(char *text,int l)
{
int f,x;
char p[80];

    f=(int)((l-strlen(text))/2);
    if((strlen(text)>=l-2)||(f<1)){
	strcpy(log_buf,text);
	return;
    }   
    for(x=0;x<f;x++)
	p[x]=' ';
    p[x]=MED_NULL;
    sprintf(log_buf,"%s%s%s",p,text,p);
}

void do_formation(struct char_data *ch, char *argument, int cmd)
{
char b[MAX_STRING_LENGTH],a[250];

    sprintf(a,"%s's FORMATION",GET_NAME(ch->master));
    center_in_logbuf(a,71);
    strcpy(b,log_buf);
    while(b[strlen(b)-1]==' ')b[strlen(b)-1]=MED_NULL;
    strcat(b+strlen(b),"\n\r");
    if(ch->master->formation[0][0])
        center_in_logbuf(GET_NAME(ch->master->formation[0][0]),20);
    else
        center_in_logbuf("----",20);
    sprintf(b+strlen(b),"[ %20s",log_buf);
    if(ch->master->formation[0][1])
        center_in_logbuf(GET_NAME(ch->master->formation[0][1]),20);
    else
        center_in_logbuf("----",20);
    sprintf(b+strlen(b)," - %20s",log_buf);
    if(ch->master->formation[0][2])
        center_in_logbuf(GET_NAME(ch->master->formation[0][2]),20);
   else
        center_in_logbuf("----",20);
    sprintf(b+strlen(b)," - %20s ]\n\r",log_buf);
    if(ch->master->formation[1][0])
        center_in_logbuf(GET_NAME(ch->master->formation[1][0]),20);
    else
        center_in_logbuf("----",20);
    sprintf(b+strlen(b),"[ %20s",log_buf);
    if(ch->master->formation[1][1])
        center_in_logbuf(GET_NAME(ch->master->formation[1][1]),20);
    else
        center_in_logbuf("----",20);
    sprintf(b+strlen(b)," - %20s",log_buf);
    if(ch->master->formation[1][2])
        center_in_logbuf(GET_NAME(ch->master->formation[1][2]),20);
    else
        center_in_logbuf("----",20);
    sprintf(b+strlen(b)," - %20s ]\n\r",log_buf);
    if(ch->master->formation[2][0])
        center_in_logbuf(GET_NAME(ch->master->formation[2][0]),20);
    else
        center_in_logbuf("----",20);
    sprintf(b+strlen(b),"[ %20s",log_buf);
    if(ch->master->formation[2][1])
        center_in_logbuf(GET_NAME(ch->master->formation[2][1]),20);
    else
        center_in_logbuf("----",20);
    sprintf(b+strlen(b)," - %20s",log_buf);
    if(ch->master->formation[2][2])
        center_in_logbuf(GET_NAME(ch->master->formation[2][2]),20);
    else
        center_in_logbuf("----",20);
    sprintf(b+strlen(b)," - %20s ]\n\r",log_buf);
    global_color=33;
    send_to_char(b,ch);
    global_color=0;
}

void do_freport(struct char_data *ch, char *argument, int cmd)
{
int x,y;
char buf[256];
char class[4];

    if(!is_formed(ch)){
	send_to_char("You are alone in your formation.\n\r",ch);
	return;
    }
    global_color=33;
    send_to_char("Your formation consists of:\n\r",ch);
    for(x=0;x<3;x++){
	for(y=0;y<3;y++){
	    if(ch->master->formation[x][y]){
                if(IS_NPC(ch->master->formation[x][y]))
		    strcpy(class,"MOB");
                else
                switch(GET_CLASS(ch->master->formation[x][y])){
                    case CLASS_THIEF:
                        strcpy(class,"THI");
                        break;
                    case CLASS_CLERIC:
                        strcpy(class,"CLE");
                        break;
                    case CLASS_MAGIC_USER:
                        strcpy(class,"MAG");
                        break;
                    case CLASS_WARRIOR:
                        strcpy(class,"WAR");
                        break;
                }
		  sprintf(buf,"[%-15s][%s][lv %2d| %4d/%4dhp %4d/%4dm %4d/%4dmv]\r\n",
		   GET_NAME(ch->master->formation[x][y]), class,
                   GET_LEVEL(ch->master->formation[x][y]),
                   GET_HIT(ch->master->formation[x][y]),
                   GET_MAX_HIT(ch->master->formation[x][y]),
                   GET_MANA(ch->master->formation[x][y]),
                   GET_MAX_MANA(ch->master->formation[x][y]),
                   GET_MOVE(ch->master->formation[x][y]),
                   GET_MAX_MOVE(ch->master->formation[x][y]));
		send_to_char(buf, ch);
/*              act(buf,FALSE,ch, 0, ch->master->formation[x][y], TO_CHAR);*/
	    }
	}
    }
    global_color=0;


}











