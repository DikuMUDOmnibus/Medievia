/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		                   *
*       Copyright (C) 1992, 1995 INTENSE Software(tm) and Mike Krause	   *
*			   All rights reserved				   						   *
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
extern void fall(struct char_data *ch);
extern char grep_text[250];
extern char *god_list[];
extern char global_color;
extern int top_of_zone_table;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct obj_data  *objs[MAX_OBJ];
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[26];
extern char wizlock;
extern struct zone_data *zone_table;

int post_office(struct char_data *ch, int cmd, char *arg);
/* external functs */
extern bool in_a_shop(struct char_data *ch);
void set_title(struct char_data *ch);
int str_cmp(char *arg1, char *arg2);
char *skip_spaces(char *string);
struct time_info_data age(struct char_data *ch);
bool is_formed(struct char_data *ch);
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);
int sort[MAX_CLANS];
bool is_in_safe(struct char_data *ch, struct char_data *victim);
void do_editroom_roomflags(struct char_data *ch, char *argument, int cmd);
extern void do_editroom_actions(struct char_data *ch, char *argument, int cmd);

extern bool IS_IN_BACK(struct char_data *ch);  
extern bool IS_IN_FRONT(struct char_data *ch);  
extern struct char_data *pick_victim(struct char_data *ch); 

struct global_clan_info_struct global_clan_info;
extern void tell_clan(int clan, char *argument);

void sort_clans(void)
{
int x,y,z,hinum,clan,place=0;

  	for(x=0;x<MAX_CLANS;x++){
  		sort[x]=MAX_CLANS+1;
  	}
	for(y=0;y<MAX_CLANS;y++){
		hinum=0;clan=0;
	    for(x=0;x<MAX_CLANS;x++){
    		if(global_clan_info.clan[x]>0){
				if(global_clan_info.eggs[x]>=hinum){
					for(z=0;z<MAX_CLANS;z++){
						if(sort[z]==global_clan_info.clan[x])break;
					}
					if(z<MAX_CLANS)
						continue;
					hinum=global_clan_info.eggs[x];
					clan=global_clan_info.clan[x];
				}		
    		}
    	}
		if(clan){
			sort[place++]=clan;
		}
	}
}

void load_clan_info(void)
{
FILE *fp=NULL;
int x;
char filename[50];
struct clan_info clan_info;

    for(x=0;x<MAX_CLANS;x++){
		sprintf(filename,"../clan/%d.clan",x+1);
		if((fp=med_open(filename,"rb"))!=NULL){
	    	open_files++;
		    if(fread(&clan_info,sizeof(clan_info),1,fp)!=0){
				global_clan_info.clan[x+1]=clan_info.clan;
				strcpy((char *)&global_clan_info.clan_name[x+1],clan_info.clan_name);
				strcpy((char *)&global_clan_info.clan_leader[x+1],clan_info.clan_leader);
				global_clan_info.eggs[x+1]=clan_info.eggs;
				global_clan_info.gold[x+1]=clan_info.gold;
	    	}
		    med_close(fp);
		    open_files--;
		}
    }
	sort_clans();
}

void do_enrollclan(struct char_data *ch, char *argument, int cmd)
{
struct clan_info clan_info;
struct char_data *victim=NULL;
FILE *fp;
int x;
char name[MAX_INPUT_LENGTH];

   if(IS_NPC(ch))return;
   if(!ch->specials.clanleader){
	send_to_char("You are not a clan leader!\n\r",ch);
	return;
   }
   if(!argument[0]){
	send_to_char("Yeah but, enroll who?\n\r",ch);
	return;
   }
   one_argument(argument,name);
   victim = get_char_room_vis(ch, name);
   if(!victim){
	send_to_char("That person is not here!\n\r",ch);
	return;
   }
   if(IS_NPC(victim)){
	send_to_char("That is not a player!\n\r",ch);
	return;
   }
   if(GET_LEVEL(victim)>31){
	send_to_char("That person is a god!\n\r",ch);
	return;
   }
   if(victim->specials.clan){
	send_to_char("That person is already a clan member!\n\r",ch);
	return;
   }
   sprintf(log_buf,"../clan/%d.clan",ch->specials.clan);
   if((fp=med_open(log_buf,"rb"))==NULL){
	send_to_char("Sorry File error!\n\r",ch);
	return;
   }
   open_files++;
   if(fread(&clan_info,sizeof(clan_info),1,fp)==0){
	send_to_char("Sorry File error!\n\r",ch);
	med_close(fp);
	open_files--;
	return;
   }
   med_close(fp);
   open_files--;
   for(x=0;x<MAX_CLAN_MEMBERS;x++)
	if(!clan_info.members[x][0])
	   break;
   if(x>=MAX_CLAN_MEMBERS){
	send_to_char("Sorry your clan is full [MAX_CLAN_MEMBERS].\n\r",ch);
	return;
   }
   strcpy(&clan_info.members[x][0],GET_NAME(victim));
   if((fp=med_open(log_buf,"wb"))==NULL){
	send_to_char("Sorry File error!\n\r",ch);
	return;
   }
   open_files++;
   if(fwrite(&clan_info,sizeof(clan_info),1,fp)==0){
	send_to_char("Sorry File error!\n\r",ch);
        med_close(fp);
   	open_files--;
	return;
   }
   med_close(fp);
   open_files--;
   victim->specials.clan=ch->specials.clan;
   victim->specials.clanleader=0;
   sprintf(log_buf,"[%s] has enrolled in the %s clan!",GET_NAME(victim),clan_info.clan_name);
   do_echo(ch,log_buf,0);
   sprintf(log_buf,"[CLAN] %s has just become a member!!!",GET_NAME(victim));
   tell_clan(ch->specials.clan,log_buf);
}



bool update_clan_eggs(int clan, int eggs)
{
struct clan_info clan_info;
FILE *fp;
 
   sprintf(log_buf,"../clan/%d.clan",clan);
   if((fp=med_open(log_buf,"rb"))==NULL){
        sprintf(log_buf,"## could not open  clan %d info in load_clan",clan);
        log_hd(log_buf);
		return(FALSE);
   }
   open_files++;
   if(fread(&clan_info,sizeof(clan_info),1,fp)==0){
	med_close(fp);
	open_files--;
        sprintf(log_buf,"## could not read clan %d info in load_clan",clan);
        log_hd(log_buf);
                
	return(FALSE);
   }
   med_close(fp);
   open_files--;

   	clan_info.eggs+=eggs;

   if((fp=med_open(log_buf,"wb"))==NULL){
		sprintf(log_buf,"## could not save clan %d in save_clan",clan);
		log_hd(log_buf);
		return(FALSE);
   }
   	open_files++;
   	if(fwrite(&clan_info,sizeof(clan_info),1,fp)==0){
        med_close(fp);
   		open_files--;
		sprintf(log_buf,"## could not save clan %d info in save_clan",clan);
		log_hd(log_buf);
		return(FALSE);
   }
   med_close(fp);
   open_files--;
   sort_clans();
   return(TRUE);
}

void do_removeclan(struct char_data *ch, char *argument, int cmd)
{
struct clan_info clan_info;
struct char_data *victim=NULL;
FILE *fp;
int x;
char name[MAX_INPUT_LENGTH];

   if(IS_NPC(ch))return;
   if(!ch->specials.clanleader&&GET_LEVEL(ch)<34){
	send_to_char("You are not a clan leader!\n\r",ch);
	return;
   }
   if(!argument[0]){
	send_to_char("Yeah but, remove who?\n\r",ch);
	return;
   }
   one_argument(argument,name);
   name[0]=UPPER(name[0]);
   victim = get_char(name);
   if(!victim){
	send_to_char("That person is not here!\n\r",ch);
	send_to_char("Will try and remove anyway, REMEMBER Use proper CASE with name.\n\r",ch);
   }else{
       if(IS_NPC(victim)){
	   send_to_char("That is not a player!\n\r",ch);
	   return;
       }
       if(victim->specials.clan!=ch->specials.clan){
	   send_to_char("That person is not a clan member!\n\r",ch);
	   return;
       }
       strcpy(name,GET_NAME(victim));
   }
   sprintf(log_buf,"../clan/%d.clan",ch->specials.clan);
   if((fp=med_open(log_buf,"rb"))==NULL){
	send_to_char("Sorry File error!\n\r",ch);
	return;
   }
   open_files++;
   if(fread(&clan_info,sizeof(clan_info),1,fp)==0){
	send_to_char("Sorry File error!\n\r",ch);
	med_close(fp);
	open_files--;
	return;
   }
   med_close(fp);
   open_files--;
   for(x=0;x<MAX_CLAN_MEMBERS;x++){
	sprintf(log_buf,"Searching... %s to %s.\n\r",name,&clan_info.members[x][0]);
	send_to_char(log_buf,ch);
	if(!str_cmp(name,&clan_info.members[x][0])){
	   send_to_char("Found!\n\r",ch);
	   break;
	}
   }

   if(x>=MAX_CLAN_MEMBERS){
	send_to_char("PLAYER NOT IN YOUR CLAN FILE!\n\rRemember to use proper Upper/lower case in name.\n\r",ch);
	return;
   }
   for(x=x;x<MAX_CLAN_MEMBERS-1;x++){
      strcpy(&clan_info.members[x][0],&clan_info.members[x+1][0]);
   }
   clan_info.members[MAX_CLAN_MEMBERS-1][0]=MED_NULL;
   sprintf(log_buf,"../clan/%d.clan",ch->specials.clan);
   if((fp=med_open(log_buf,"wb"))==NULL){
	send_to_char("Sorry File error!\n\r",ch);
	return;
   }
   open_files++;
   if(fwrite(&clan_info,sizeof(clan_info),1,fp)==0){
	send_to_char("Sorry File error!\n\r",ch);
	med_close(fp);
	open_files--;
	return;
   }
   med_close(fp);
   open_files--;
   if(victim)victim->specials.clan=0;
   sprintf(log_buf,"[%s] has been removed from the %s clan!",name,clan_info.clan_name);
   do_echo(ch,log_buf,0);
   sprintf(log_buf,"[CLAN] %s has just been kicked out of the Clan!!!",name);
   tell_clan(ch->specials.clan,log_buf);
}


void do_showclans(struct char_data *ch, char *argument, int cmd)
{
int x,clan;
char clan_buf[MAX_STRING_LENGTH];
struct clan_info clan_info;
FILE *fp;

    if(!argument[0]){
	sprintf(clan_buf, "%s\n\r[   EGGS   ][##][%15s] %s\n\r",
		WHT(ch),
		"Clan Leader",
		"____Clan Name____");
	for(x=0;x<MAX_CLANS;x++){
	   if(sort[x]<(MAX_CLANS+1)){
	      sprintf(clan_buf,"%s%s[%10d][%2d][%15s] %s%s\n\r",
		clan_buf,
		MAG(ch),
	        global_clan_info.eggs[sort[x]],
	        global_clan_info.clan[sort[x]],
		&global_clan_info.clan_leader[sort[x]][0],
		&global_clan_info.clan_name[sort[x]][0],
		NRM(ch));
	   }
	}
	page_string(ch->desc, clan_buf, 1);
	return;
    }else{
       	clan=atoi(argument);
       	if(clan<0||clan>MAX_CLANS){
	   send_to_char("Not a valid clan number!\n\r",ch);
	   return;
       	}
   	sprintf(log_buf,"../clan/%d.clan",clan);
   	if((fp=med_open(log_buf,"rb"))==NULL){
	   send_to_char("No such clan!\n\r",ch);
	   return;
   	}
	open_files++;
   	if(fread(&clan_info,sizeof(clan_info),1,fp)==0){
	   send_to_char("Sorry File error!\n\r",ch);
	   med_close(fp);
	   open_files--;
	   return;
   	}
   	med_close(fp);
	open_files--;
	sprintf(clan_buf,"\n\r%sClan Name: %s\n\r",WHT(ch), clan_info.clan_name);
	sprintf(clan_buf,"%s%sClan Leader: %s\n\r\n\r",
		clan_buf, 
		RED(ch), 
		clan_info.clan_leader);

	for(x=0;x<MAX_CLAN_MEMBERS;x++){
	   if(clan_info.members[x][0]){
	      sprintf(clan_buf,"%s%sClan Member: %s\n\r",
		clan_buf,
		GRN(ch),
		&clan_info.members[x][0]);
	   }else break;
	}
	page_string(ch->desc, clan_buf, 1);
    }
}

void tell_clan(int clan, char *argument)
{
    struct descriptor_data *to=NULL;
    int x;
    char buf[MAX_STRING_LENGTH];

    if(!argument[0])return;
    if(!clan)return;
    for (x = 0; *(argument + x) == ' '; x++); 
    for(to=descriptor_list; to; to=to->next){
 	if(!to->connected&&to->character->specials.clan==clan){
	    global_color=36;
	    sprintf(buf,"%s\n\r",argument);
            send_to_char(buf,to->character); 
	    global_color=0;
	}
    }
}

void do_clanwhere(struct char_data *ch, char *argument, int cmd)
{
struct descriptor_data *to=NULL;
char buf[MAX_INPUT_LENGTH];
    
    if(!ch->specials.clan){
	send_to_char("You are not a Clan member!\n\r",ch);
	return;
    }
    sprintf(buf,"\n\r[%15s] ________WHERE________\n\r","PLAYER NAME");
    global_color=1;
    send_to_char(buf,ch);
    for(to=descriptor_list; to; to=to->next){
 	if(to->character && (to->connected == CON_PLAYING)&&
	   (to->character->in_room != NOWHERE)&&
	   (GET_LEVEL(to->character) < 33)&&
	    to->character->specials.clan==ch->specials.clan){
            sprintf(buf,"[%15s] %s\n\r",GET_NAME(to->character),
			world[to->character->in_room]->name);
	    global_color=32;	    
            send_to_char(buf,ch); 
        }
    }
    global_color=0;
}
void do_transclan(struct char_data *ch, char *argument, int cmd)
{
struct descriptor_data *to=NULL;
    
    if(!ch->specials.clan){
	send_to_char("You are not a Clan member use (LEADCLAN)!\n\r",ch);
	return;
    }
    for(to=descriptor_list; to; to=to->next){
 	if(to->character && (to->connected == CON_PLAYING)&&
		to->character->specials.clan==ch->specials.clan){
	    do_trans(ch,GET_NAME(to->character),9);
        }
    }
}
void do_logclan(struct char_data *ch, char *argument, int cmd)
{
struct descriptor_data *to=NULL;
    
    if(!ch->specials.clan){
	send_to_char("You are not a Clan member use (LEADCLAN)!\n\r",ch);
	return;
    }
    for(to=descriptor_list; to; to=to->next){
 	if(to->character && (to->connected == CON_PLAYING)&&
		to->character->specials.clan==ch->specials.clan){
	    do_log(ch,GET_NAME(to->character),9);
        }
    }
}
void do_clanwho(struct char_data *ch, char *argument, int cmd)
{
struct descriptor_data *to=NULL;
char clan_buf[MAX_STRING_LENGTH];
int x, clan=0;

    if (*argument) {
       	clan=atoi(argument);
       	if (clan < 1 || clan > MAX_CLANS){
	   send_to_char("Not a valid clan number!\n\r",ch);
	   return;
       	}
    }
    sprintf(clan_buf,"\n\r%s[%15s] [##] ________CLAN NAME_______\n\r",
	WHT(ch),
	"PLAYER NAME");
    for(to=descriptor_list; to; to=to->next){
 	if(!to->connected&&to->character->specials.clan){
	    if (GET_LEVEL(to->character) > 33)
		continue;
	    for(x=0;x<MAX_CLANS;x++)
		if(global_clan_info.clan[x]==to->character->specials.clan)
		   break;
	    if (clan == 0 || clan == to->character->specials.clan)
	    sprintf(clan_buf,"%s%s[%15s] [%2d] %s\n\r",
		clan_buf,
		GRN(ch),
		GET_NAME(to->character),
		to->character->specials.clan,
		&global_clan_info.clan_name[x][0]);
        }
    }
    page_string(ch->desc, clan_buf, 1);
    global_color=0;
}

void do_clantalk(struct char_data *ch, char *argument, int cmd)
{
char buf[MAX_INPUT_LENGTH+100];
char CorruptBuf[MAX_INPUT_LENGTH];
int iPercent=0;
int distance;
struct descriptor_data *to=NULL;
void CorruptString(char *szpNew, char *szpString, int iPercent);
int iHoloChDistance(struct char_data *ch, struct char_data *vict);

    if(IS_NPC(ch))return;
    if(!ch->specials.clan){
	send_to_char("You are not a clan member!\n\r",ch);
	return;
    }
    if(!argument[0]){
	send_to_char("Yes, but say what?\n\r",ch);
	return;
    }
    for(to=descriptor_list; to; to=to->next){
 	if(!to->connected&&to->character->specials.clan==ch->specials.clan){
	    global_color=36;
	    distance = iHoloChDistance(to->character, ch);
	    if(distance < 600) {
	    	iPercent = distance / 20;
	    	CorruptString(CorruptBuf, argument, iPercent);
    	    	if(ch->specials.clanleader)
        		sprintf(buf,"[CLAN]: Leader %s says, '%s'.\r\n",GET_NAME(ch),CorruptBuf);
    	    	else
        		sprintf(buf,"[CLAN]: %s says, '%s'.\r\n",GET_NAME(ch),CorruptBuf);

            	send_to_char(buf,to->character); 
	   	global_color=0;
	    }
	}
    }

}

void do_lead_clan(struct char_data *ch, char *argument, int cmd)
{
int clan=0;
char number[MAX_INPUT_LENGTH];
   if(IS_NPC(ch))return;
   if(!argument[0]){
	send_to_char("Yes but which clan Number?",ch);
	return;
   }
   one_argument(argument,number);
   clan=atoi(number);
   if(clan<0||clan>MAX_CLANS){
	send_to_char("Not a valid clan number must be 1 to MAX_CLANS.\n\r",ch);
	return;
   }
   ch->specials.clanleader=clan;
   ch->specials.clan=clan;
   if(!clan)
	send_to_char("DONE, you are no longer a clan leader.\n\r",ch);
   else
	{
	sprintf(log_buf,"%s leads clan %d.",GET_NAME(ch),clan);
	log_hd(log_buf);
	send_to_char("Done, you are now also a leader of that clan!\n\r",ch);
	}
}

void do_make_clan(struct char_data *ch, char *argument, int cmd)
{
int clan_number;
int x;
char buf[MAX_STRING_LENGTH];
char number[MAX_STRING_LENGTH];
char clan_leader[MAX_STRING_LENGTH];
char clan_name[MAX_STRING_LENGTH];
struct char_data *victim=NULL;
FILE *fp=NULL;
char filename[MAX_STRING_LENGTH];
struct clan_info clan_info;

   if(IS_NPC(ch))return;

   if(!argument[0]){
	send_to_char("SYNTAX: makeclan clan# leader_name clan_name\n\r",ch);
	return;
   }
   half_chop(argument, number, buf);
   if(!buf){
	send_to_char("SYNTAX: makeclan clan# leader_name clan_name\n\r",ch);
	return;
   }
   half_chop(buf, clan_leader, clan_name);
   if(!clan_leader||!clan_name){
	send_to_char("SYNTAX: makeclan clan# leader_name clan_name\n\r",ch);
	return;
   }
   clan_number=atoi(number);
   if(clan_number<1||clan_number>MAX_CLANS){
	send_to_char("clan_number must be from 1 to MAX_CLANS\n\r",ch);
	return;
   }
   if (!(victim = get_char_vis(ch,clan_leader))){
        send_to_char("No-one by that name around.\n\r",ch);
	return;
   }
   if(victim->specials.clan){
	send_to_char("That player is already a member of a clan!\n\r",ch);
	return;
   }
   if(GET_LEVEL(victim)<15){
	send_to_char("A leader must be level 15+\n\r",ch);
	return;
   }
   sprintf(filename,"../clan/%s.clan",number);
   if((fp=med_open(filename,"rb"))!=NULL){
	med_close(fp);
	send_to_char("That clan already exists!\n\r",ch);
	return;
   }
   if(GET_GOLD(victim)<75000000){
	send_to_char("Leader does not have 75Million gold!\n\r",ch);
	return;
   }   
   clan_info.clan=clan_number;   
   strcpy(clan_info.clan_name,clan_name);
   strcpy(clan_info.clan_leader,GET_NAME(victim));
   clan_info.eggs=0;
   clan_info.gold=0;
   clan_info.clanhall_medievia=0;
   clan_info.clanhall_trellor=0;
   for(x=0;x<10;x++)
   		clan_info.expansion[x]=0;
   for(x=0;x<MAX_CLAN_MEMBERS;x++)
		clan_info.members[x][0]=MED_NULL;
   if((fp=med_open(filename,"wb"))==NULL){
	send_to_char("That clan already exists!\n\r",ch);
	return;
   }
   open_files++;
   if(fwrite(&clan_info,sizeof(clan_info),1,fp)==0){
	send_to_char("ERROR WRITING TO FILE!\n\r",ch);
	med_close(fp);
	open_files--;
	return;
   }
   med_close(fp);
   open_files--;
   GET_GOLD(victim)-=75000000;
   victim->specials.clanleader=clan_number;
   victim->specials.clan=clan_number;
   sprintf(log_buf,"THE CLAN: %s has just been created!",clan_name);
   do_echo(ch,log_buf,0);
   sprintf(log_buf,"THE CLAN LEADER: %s",clan_leader);
   do_echo(ch,log_buf,0);
   load_clan_info();
}

