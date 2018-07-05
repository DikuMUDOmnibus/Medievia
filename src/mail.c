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
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/*   external vars  */
extern FILE *hold2;
extern char *GET_REAL_NAME(struct char_data *ch);
struct index_data *mob_index;
extern int number_of_players();
extern struct obj_data  *objs[MAX_OBJ];
extern struct obj_data *deteriorate(struct obj_data *obj);
extern unsigned long int connect_count;
extern int number_of_rooms;
extern int number_of_zones;
extern struct zone_data *zone_table;
extern char global_color;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern put_obj_in_store(struct obj_data *obj, struct char_data *ch, FILE *fpsave);
extern bool in_a_shop(struct char_data *ch);
extern char *fread_paper(FILE *fl);
extern int top_of_world;
extern int top_of_zone_table;
extern int VERSIONNUMBER;
extern int weigh(struct obj_data *obj);

int post_office(struct char_data *ch, int cmd, char *arg);


void check_mail(struct char_data *ch)
{
char text[100],name[100];
FILE *fh;

    strcpy(name,GET_NAME(ch));
    sprintf(text,"%s/%c/%s.m", SAVE_DIR,LOWER(name[0]),name);
    if((fh=med_open(text, "r"))!=NULL){
    	open_files++;
    	send_to_char("\n\r[MAIL] You have mail waiting at the post office.",ch);
    	send_to_char("\n\r", ch);
    	ch->specials.mail_waiting=1;
    	med_close(fh);
    	open_files--;
    }else{
    	ch->specials.mail_waiting=0;
    }
}


void mail_to_vryce_and_assistants(struct char_data *ch,char *obj)
{
char buf[MAX_INPUT_LENGTH];

    sprintf(buf,"%s Io",obj);
    post_office(ch,999,buf);
    sprintf(buf,"%s Vryce",obj);
    post_office(ch,999,buf);
    sprintf(buf,"%s Shalafi",obj);
    post_office(ch,999,buf);
    send_to_char("Mail has been forwarded to Vryce's assistants.\n\r",ch);
}

int post_office(struct char_data *ch, int cmd, char *arg)
{
char objname[MAX_INPUT_LENGTH];
char name[MAX_INPUT_LENGTH];
char text[MAX_STRING_LENGTH];
FILE *fh=NULL, *fhn=NULL;
struct obj_data *obj;
struct char_data *victim;
int nr,j;
long tc;
struct tm *t_info=NULL;
 

if (IS_NPC(ch))
	{
	send_to_char("Sorry, you can't use the post office!\n\r",ch);
	return(0);
	}
    switch(cmd){
	case 100:  /* send */
	case 999:  /* used by godsend command */
	    if(GET_LEVEL(ch)<6){
		send_to_char("Sorry, you must be level 6 to send mail.\n\r",ch);
		return(1);
	    }
	    half_chop(arg, objname, name);
	    if(!objname[0]||!name[0]){
		send_to_char("Syntax> send <obj> <player>\n\r", ch);
		return(1);
	    }
	    if(cmd!=999&&!strcmp("Vryce",name)
			&&!IS_PLAYER(ch,"Io")
			&&!IS_PLAYER(ch,"Shalafi")
			&&!IS_PLAYER(ch,"Elywyn")
			){
		mail_to_vryce_and_assistants(ch,objname);
		return(TRUE);
	    }
	    sprintf(text,"%s/%c/%s", SAVE_DIR, LOWER(name[0]),name);
	    if((fh=med_open(text, "r"))==NULL){
		send_to_char("That player doesn't exist.\n\rYou must type the player name with the proper upper/lower case.\n\r", ch);
		return(1);
	    }
	    med_close(fh);fh=NULL;
	    if(!(obj=get_obj_in_list_vis(ch, objname, ch->carrying))){
		send_to_char("You don't seem to have that!\n\r",ch);
		return(1);
	    }
	    if(IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)
		&&strcmp(GET_NAME(ch),"Vryce")){
		
		if(obj->item_number==10||obj->item_number==16)
		    send_to_char("You feel VERTIGO as the gods cringe in saddness of your thoughts.\n\r",ch);
		else
		    send_to_char("You can't let go of it! YEECH!\n\r", ch);
		return(1);
	    }
	    if(	(obj->item_number==1399)
			|| strstr(obj->short_description,"A bright ball of light")
	    	|| (obj->obj_flags.type_flag==ITEM_CONTAINER)
			|| (obj->obj_flags.type_flag==ITEM_KEY)
			|| (obj->obj_flags.type_flag==ITEM_SCROLL)
			|| (obj->obj_flags.type_flag==ITEM_WAND)
			|| (obj->obj_flags.type_flag==ITEM_STAFF)
			|| (obj->obj_flags.type_flag==ITEM_POTION)
			|| IS_SET(obj->obj_flags.extra_flags, ITEM_NO_RENT)
			)
			{
			global_color=31;
			send_to_char("Safety regulations do not allow us to ship that.\n\r",ch);
			global_color=0;
			return(TRUE);
			}
	    if(cmd!=999)
	    if(GET_GOLD(ch)<(obj->obj_flags.cost_per_day*2))
			{
			sprintf(text,"You don't have the %d coins it takes to send %s to %s.\n\r", obj->obj_flags.cost_per_day*2, objname, name);
			send_to_char(text,ch);
			return(1);
            }
	    if(obj->obj_flags.type_flag == ITEM_NOTE){
		if(strlen(obj->action_description)>8000){
		    send_to_char("Note to long.\n\r",ch);
		    return(1);
		}
	    }
	    sprintf(text,"%s/%c/%s.m", SAVE_DIR, LOWER(name[0]),name);
	    if((fh=med_open(text, "a"))==NULL){
		log_hd("Error opening/creating mail file");
		send_to_char("Sorry due to a backlog, we cannot serve you now.\n\r", ch);
		return(1);
	    }
	    open_files++;
	    if(!put_obj_in_store(obj, ch, fh)){
		send_to_char("Sorry, we cannot serve you now due to backlog.\n\r", ch);
	    	med_close(fh);fh=NULL;
	    	open_files--;
		return(1);
	    }
	    med_close(fh);fh=NULL;
		open_files--;
		if(obj->obj_flags.type_flag == ITEM_NOTE)
			{
			sprintf(text,"%s/%c/%s.n", SAVE_DIR, LOWER(name[0]),name);
			if((fh=med_open(text, "a"))==NULL)
			{
				log_hd("Error opening/creating mail file");
				send_to_char("Sorry due to a backlog, we cannot serve you now.\n\r", ch);
				return(1);
				}
			if(strlen(obj->action_description)>5)
				{
    			tc = time(0);
    			t_info = localtime(&tc);
				sprintf(text,"%s\nMailed by: %s\nDate    : %d/%d/%d %2d:%2d\n~\n", 
					obj->action_description,
					GET_REAL_NAME(ch), t_info->tm_mon+1, t_info->tm_mday,
					t_info->tm_year, t_info->tm_hour, t_info->tm_min);
				fputs(text, fh);
				}
			else
				{
				fputs("  ~\n",fh);
	       		}
			med_close(fh);fh=NULL;
			open_files--;
	    	}
	    if(cmd!=999)
	    GET_GOLD(ch)-=obj->obj_flags.cost_per_day*2;
	    sprintf(text, "%s sent to %s at a cost of %d.\n\r",obj->short_description, name, obj->obj_flags.cost_per_day*2);
	    send_to_char(text, ch);
	    sprintf(text, "You see $n sending %s to %s.\n\r", obj->short_description, name);
	    act(text,TRUE, ch, 0, 0, TO_ROOM);
	    sprintf(text, "You see %s sending %s to %s.", GET_NAME(ch), obj->short_description, name);
	    log_hd(text);
	    if(cmd!=999)
	       extract_obj(obj);
    	    victim=get_char(name);
	    if(victim)
			victim->specials.mail_waiting=1;
	    SAVE_CHAR_OBJ(ch,-20);
	    return(1);
            break;
	case 101:  /* receive */
	    strcpy(name,GET_NAME(ch));
	    sprintf(text,"%s/%c/%s.m", SAVE_DIR, LOWER(name[0]),name);
	    if((fh=med_open(text, "r"))==NULL){
		send_to_char("Sorry, nothing is here for you today\n\r", ch);
		return(1);
	    }
	    sprintf(log_buf,"(%s)recieving mail at room %d",GET_NAME(ch),ch->in_room); 
	    log_hd(log_buf);
	    open_files++;
 	    while(!feof(fh)){
		struct obj_file_elem object;
		fread(&object, sizeof(object),1, fh);
		if(ferror(fh)) goto LError;
		if(feof(fh)) break;
		nr=real_object(object.item_number);
		obj=read_object(nr, 0);
		if(!obj){
		    sprintf(log_buf,"## in post_office recieve, object #%d doesn't exist",nr);
		    log_hd(log_buf);
		    send_to_char("Sorry, we lost your stuff!\n\r",ch);
		    if(fh){
			med_close(fh);
			open_files--;
		    }
		    if(fhn){
			fclose(fhn);fhn=NULL;
			hold2=fopen("../lib/hold2.dat","w");
		    }
		    sprintf(text,"%s/%c/%s.m", SAVE_DIR, LOWER(name[0]),name);
		    unlink(text);
		    sprintf(text,"%s/%c/%s.n", SAVE_DIR, LOWER(name[0]),name);
		    unlink(text);
		    return(1);
		}
		obj->obj_flags.value[0]=object.value[0];
		obj->obj_flags.value[1]=object.value[1];
		obj->obj_flags.value[2]=object.value[2];
		obj->obj_flags.value[3]=object.value[3];
		obj->obj_flags.extra_flags=object.extra_flags;
		obj->obj_flags.weight=object.weight;
		obj->obj_flags.timer=object.timer;
		obj->obj_flags.eq_level=object.eq_level;
		obj->obj_flags.bitvector=object.bitvector;
		obj->iValueWeight=object.iWeight;
		obj->iWeightVersion=object.iWeightVersion;
		obj->iBornDate = object.iItemBorn;
		obj->iLastDet = object.iLastDet;
		obj->iDetLife = object.iItemLasts;
		
		if(!obj->iValueWeight||obj->iWeightVersion!=VERSIONNUMBER){
			obj->iValueWeight=weigh(obj);
			obj->iWeightVersion=VERSIONNUMBER;
			obj->iBornDate = (int) time(NULL);
			obj->iDetLife = objs[obj->item_number]->iDetLife;
			obj->iLastDet = 0;
		}
		
		for(j=0;j<MAX_OBJ_AFFECT;j++)
		    obj->affected[j]=object.affected[j];
		if(obj->obj_flags.type_flag == ITEM_NOTE){
		   if(fhn==NULL){
		     strcpy(name,GET_REAL_NAME(ch));
		     sprintf(text,"%s/%c/%s.n", SAVE_DIR,LOWER(name[0]),name);
		     fclose(hold2);hold2=NULL;
		     if((fhn=fopen(text, "r"))==NULL){
			send_to_char("Sorry, nothing is here for you today\n\r", ch);
			if(fh)med_close(fh);
			hold2=fopen("../lib/hold2.dat","w");
			return(1);
		     }
		   }
		   if(obj->action_description!=objs[obj->item_number]->action_description)
		       my_free(obj->action_description);
		   obj->action_description=fread_string(fhn);		
		}
		/* Object DETING */

		obj = deteriorate(obj);
		obj_to_char(obj,ch);
		sprintf(text,"You receive %s.\n\r",obj->short_description);
		send_to_char(text,ch); 
	        sprintf(text, "$n receives %s.\n\r", obj->short_description);
 	        act(text,TRUE, ch, 0, 0, TO_ROOM);
	    }	    
	    goto LSuccess;
LError:
	    log_hd("error reading object in mud postoffice");
	    if(fh!=NULL)
			{
			med_close(fh);fh=NULL;
			open_files--;
			}
	    if(fhn!=NULL)
			{
			fclose(fhn);
			hold2=fopen("../lib/hold2.dat","w");
			}
	    return(1);

LSuccess:
	    if(fh!=NULL){
		med_close(fh);
		open_files--;
	    }
	    strcpy(name,GET_REAL_NAME(ch));
	    sprintf(text,"%s/%c/%s.m",SAVE_DIR,LOWER(name[0]),name);
	    unlink(text);
	    if(fhn!=NULL){
		fclose(fhn);
		hold2=fopen("../lib/hold2.dat","w");
	    	sprintf(text,"%s/%c/%s.n", SAVE_DIR,LOWER(name[0]),name);
	    	unlink(text);
	    }
	    ch->specials.mail_waiting=0;
		SAVE_CHAR_OBJ(ch,-20);
	    return(1);
	    break;
    }
    return(0);
}
