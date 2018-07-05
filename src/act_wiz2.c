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
#include <dirent.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#ifndef __FreeBSD__
#include <malloc.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

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
extern int port;
extern char global_color;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct zone_data zone_table_array[MAX_ZONE];  
extern struct zone_data *zone_table;  
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct char_data *mobs[MAX_MOB];
extern struct obj_data *objs[MAX_OBJ];
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern struct ban_t *ban_list;
extern char wmfha[1300];
extern char downtxt[1000];
extern int slot_rooms[];
extern char use_hash_table;

/* external functs */
extern void write_filtered_text(FILE *fh, char *text);
/*int system(char *);*/
extern void page_string(struct descriptor_data *d, char *str, int keep_internal);
void save_rooms(struct char_data *ch, int zone);
void set_title(struct char_data *ch);
int str_cmp(char *arg1, char *arg2);
struct time_info_data age(struct char_data *ch);
void sprinttype(int type, char *names[], char *result);
void sprintbit(long vektor, char *names[], char *result);
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);
extern struct mallinfo mallinfo();
extern void sort_descriptors(void);

extern FILE *obj_f, *mob_f;
extern char use_memory_debug;
extern struct global_clan_info_struct global_clan_info;
extern void load_clan_info(void);
extern char daytimedown;
extern struct obj_data *tweak(struct obj_data *);

bool update_trivia_file(struct char_data *);
void clean_upnwrite_text(FILE *, char *);

void do_approve(struct char_data *stpCh, char *szpArgument, int iCmd)
{
	char szName[MAX_INPUT_LENGTH], szBuf[MAX_STRING_LENGTH];
	struct char_data *stpVict; 
	struct descriptor_data *stpI;
	int iLoop=0;

	if(IS_NPC(stpCh)) return;
	    
	one_argument(szpArgument, szName);
	*szBuf = '\0';

	
	if(!*szName) {
			  for(stpI = descriptor_list; stpI; stpI = stpI->next) {
					if(stpI->character->p)
					if(!IS_SET(stpI->character->p->iFlags, FLG_APPROVED)) {
						 sprintf(szBuf + strlen(szBuf),"%-13s",GET_NAME(stpI->character));
						 iLoop++;
						 if( (iLoop % 5) == 0)
						 	strcat(szBuf,"\r\n");
					 }
				}
				if(!*szBuf) 
						send_to_char("All Chars approved.\r\n",stpCh);
				else
				page_string(stpCh->desc,szBuf,1);
				return;
	}
	
	stpVict = get_char_vis(stpCh, szName);
	
	if(!stpVict) {
		send_to_char("No such char.\r\n",stpCh);
		return;
	}
	if(IS_NPC(stpVict)) {
		send_to_char("Mobs can't be approved.\r\n",stpCh);
		return;
	}
	if(IS_SET(stpVict->p->iFlags, FLG_APPROVED)) {
		send_to_char("That player is allready approved.\r\n",stpCh);
		return;
	}
	SET_BIT(stpVict->p->iFlags, FLG_APPROVED);
	send_to_char("Player Name Approved.\r\n",stpCh);
	return;
}
						
	
void do_rename(struct char_data *stpCh, char *szpArgument, int iCmd)
{
	char szBuf[400],szBuf2[400],szFileName[250];
	char szCurrent[MAX_INPUT_LENGTH],szNew[MAX_INPUT_LENGTH];
	char szOldFileName[250];
	struct char_data *stpVict;
	FILE *fh=NULL;
	char c,olddir;

#if !defined(__FreeBSD__) && !defined(linux) && !defined(sun)
	int rename(char *path1, char *path2);
#endif
	
	szpArgument = one_argument(szpArgument, szCurrent);
	one_argument(szpArgument, szNew);
	
	if(!szNew[0]||!szCurrent[0]){
		send_to_char("Syntax: rename oldname newname.\r\n",stpCh);
		return;
	}
	if(strlen(szNew)<=3) {
		send_to_char("New name not long enough.\n\r",stpCh);
		return;
	}
	stpVict = get_char(szCurrent);
	if(!stpVict) {
		send_to_char("No such char\r\n",stpCh);
		return;
	}
	if(IS_NPC(stpVict)) {
		send_to_char("You can't rename a NPC\r\n",stpCh);
		return;
	}
	if(IS_DEAD(stpVict)) {
		send_to_char("Not to a deadman!\r\n",stpCh);
		return;
	}
	if( (GET_LEVEL(stpVict)>31) && !IS_PLAYER(stpCh,"Vryce")) {
		send_to_char("You may not change a gods name.\r\n",stpCh);
		return;
	}
	if(get_char(szNew)) {
		send_to_char("That name is in use.\r\n",stpCh);
		return;
	}
	olddir = *GET_NAME(stpVict);
	c=LOWER(szNew[0]);
	szNew[0] = UPPER(szNew[0]);
	sprintf( szFileName, "%s/%c/%s", SAVE_DIR, c, szNew);
	sprintf( szOldFileName, "%s/%c/%s",SAVE_DIR, LOWER(olddir), GET_NAME(stpVict));
	fh = med_open(szFileName, "r");		
	if(fh) {
		send_to_char("The new name is allready in use.\r\n",stpCh);
		med_close(fh);
		return;
	}
	fh = med_open(szOldFileName, "r");
	if(!fh) {
		send_to_char("File Error: No such file!\r\n",stpCh);
		return;
	}
	med_close(fh);
	sprintf( log_buf, "## %s change [%s]'s name to [%s].", GET_NAME(stpCh), GET_NAME(stpVict), szNew);
	log_hd( log_buf );
	sprintf(szBuf,"%s.n",szOldFileName);
	fh = med_open(szBuf,"r");
	if(fh) {
		med_close(fh);
		sprintf(szBuf2,"%s.n",szFileName);
		if(rename(szBuf,szBuf2) == -1) {
			send_to_char("File Error on the .n File.\r\n",stpCh);
			return;
		}
	}
	sprintf(szBuf,"%s.m",szOldFileName);
	fh = med_open(szBuf,"r");
	if(fh) {
		med_close(fh);
		sprintf(szBuf2,"%s.m",szFileName);
		if(rename(szBuf,szBuf2) == -1) {
			send_to_char("File Error on the .m File.\r\n",stpCh);
			return;
		}
	}
	sprintf(szBuf,"%s.atm",szOldFileName);
	fh = med_open(szBuf,"r");
	if(fh) {
		med_close(fh);
		sprintf(szBuf2,"%s.atm",szFileName);
		if(rename(szBuf,szBuf2) == -1) {
			send_to_char("File Error on the .atm File.\r\n",stpCh);
			return;
		}
	}
	sprintf(szBuf,"%s.usi",szOldFileName);
	fh = med_open(szBuf,"r");
	if(fh) {
		med_close(fh);
		sprintf(szBuf2,"%s.usi",szFileName);
		if(rename(szBuf,szBuf2) == -1) {
			send_to_char("File Error on the .usi File.\r\n",stpCh);
			return;
		}
	}
	sprintf(szBuf,"%s.usi2",szOldFileName);
	fh = med_open(szBuf,"r");
	if(fh) {
		med_close(fh);
		sprintf(szBuf2,"%s.usi2",szFileName);
		if(rename(szBuf,szBuf2) == -1) {
			send_to_char("File Error on the .usi2 File.\r\n",stpCh);
			return;
		}
	}
	sprintf(szBuf,"%s.freight",szOldFileName);
	fh = med_open(szBuf,"r");
	if(fh) {
		med_close(fh);
		sprintf(szBuf2,"%s.freight",szFileName);
		if(rename(szBuf,szBuf2) == -1) {
			send_to_char("File Error on the rename File.\r\n",stpCh);
			return;
		}
	}
	/* Update Name */
	GET_NAME(stpVict) = my_free(GET_NAME(stpVict));
	GET_NAME(stpVict) = str_dup(szNew);
	save_char_obj( stpVict );
	/* Wipe the Old pfile */
	unlink(szOldFileName);
	send_to_char("Rename: Success.\r\n",stpCh);
	return;
}
	

void do_hint(struct char_data *ch, char *argument, int cmd){
	char *values[]={
	    "add","edit","delete","\n"
	};
	char my_buf[MAX_INPUT_LENGTH];
	char hint_num[20];
	int hint=0;
	int number=0;
	extern int top_trivia;
	extern char *trivia[];

	argument=one_argument(argument, my_buf);

	if(!*my_buf){
	    send_to_char("Syntax:\r\n",ch);
	    send_to_char("hint add <new hint info>\r\n", ch);
	    send_to_char("hint edit <hint#>\r\n", ch);
	    send_to_char("hint delete <hint#>\r\n", ch);
	    return;
	}

	if((hint=old_search_block(my_buf, 0, strlen(my_buf), values, 1)) < 0){	
	    send_to_char("Invalid hint command, try 'hint' for syntax help.\r\n", ch);
	    return;
	}

	switch(hint){
	    case 1:  /*add*/
		if(top_trivia>=MAX_TRIVIA){
		    send_to_char("Too many hint topics!\r\n", ch);
		    return;
		}
		if(argument[0]==' ')argument++;
		trivia[top_trivia]=strdup(argument);
		top_trivia++;
		if(!update_trivia_file(ch)){
		    send_to_char("Hmmm.. problem writing out the new hint file.\r\n", ch);
		    log_hd("## Could not update the hint file!");
		    break;
		}
		send_to_char("Hint topic added.\r\n", ch);
		break;
	    case 2:  /*edit*/
		send_to_char("Editing hint\n\r", ch);
		break;
	    case 3:
		one_argument(argument, hint_num);
		if(!is_number(hint_num)){
		    send_to_char("You need to specify a valid number.\r\n", ch);
		    break;
		}
		number=atoi(hint_num);
		if((number>=top_trivia)||(number<0)){
		    send_to_char("Invalid hint number, try again.\r\n", ch);
		    break;
		}
		if(number==(top_trivia-1))
		    my_free(trivia[number]);
		else{
		    while(trivia[number+1]){
		    	trivia[number]=trivia[number+1];
			number++;
		    }
		    my_free(trivia[number+1]);
		}
		top_trivia--;
		update_trivia_file(ch);
		send_to_char("Hint deleted.\r\n", ch);
		break;
	    default:
	}
}

bool update_trivia_file(struct char_data *ch){
	FILE *out;
	char current[MAX_INPUT_LENGTH];
	extern char *trivia[];
	extern int top_trivia;
	int counter=0;

	send_to_char("Updating hint file.\r\n", ch);
	sprintf(current, "../lib/%s", TRIVIA_FILE);

	if(!(out=med_open(current, "w"))){
	    log_hd("## Couldn't write out new hint file!.");
	    send_to_char("Problem writing out new hint file.\r\n", ch);
	    return(FALSE);
	}

	send_to_char("Writing out new hint file.\r\n", ch);
	for(counter=0;counter<top_trivia;counter++){
	    clean_upnwrite_text(out, trivia[counter]);
	}

	clean_upnwrite_text(out, "~");

	send_to_char("Done!\r\n", ch);
	med_close(out);
	return(TRUE);
}

void clean_upnwrite_text(FILE *fh, char *text){
    char *filtered;
    int x=0,f=0;

    if(text){
      CREATE(filtered,char,strlen(text)+3);
	  while(text[0]==' ')text++;
      while(text[x]){
          if((text[x]=='\r') || (text[x]=='\n') || (text[x]=='~'))x++;
          else
             filtered[f++]=text[x++];
      }
      filtered[f++]='~';filtered[f++]='\n';filtered[f]=MED_NULL;
      fprintf(fh,filtered);
      filtered = my_free(filtered);
    }else{
       fprintf(fh,"~\n");
    }
}

void do_tweak(struct char_data *ch, char *argument, int subcmd){
    char obj_name[255];
    struct obj_data *obj_object;

    one_argument(argument, obj_name);
    if(!*obj_name){
	send_to_char("You must specify an object to tweak.\r\n", ch);
	return;
    }
    obj_object = get_obj_in_list_vis(ch, obj_name, ch->carrying);
    if(!obj_object){
	send_to_char("You don't seem to have one.\r\n", ch);
	return;
    }
    obj_object=tweak(obj_object);
}

void do_trackset(struct char_data *ch, char *argument, int subcmd)
{
struct char_data *hunted=NULL, *hunter=NULL;
char name[100], name2[100];
char my_buf[300];

  argument = one_argument(argument,name);
  one_argument(argument,name2);

  if(!*name || !*name2) {
	send_to_char("Not enough arguments.\r\ntrackset <hunted> <hunter>\r\n",ch);
	return;
  }
  
  hunted = get_char_vis(ch, name);
  hunter = get_char_vis(ch,name2);

  if(!hunted || !hunter) {
	send_to_char("No such person or mobile.\r\n",ch);
	return;
  }
  if(!IS_NPC(hunter) || IS_NPC(hunted)) {
	send_to_char("You can not set PC's to track or NPC's to be hunted.\r\n",ch);  
	return;
  }
  hunter->specials.hunting = hunted;
  sprintf(my_buf,"Okay, %s is now being hunted by %s.\r\n",GET_NAME(hunted),GET_NAME(hunter));  
  send_to_char(my_buf,ch);

}

void do_savechar(struct char_data *ch, char *argument, int subcmd){
    char name[100];
    char my_buf[MAX_INPUT_LENGTH];
    struct char_data *savee;

    one_argument(argument, name);
    if(!*name){
	send_to_char("Ok, but who do you want to save?\r\n", ch);
	return;
    }

    savee=get_char_vis(ch, name);
    if(!savee){
	sprintf(my_buf, "No one named '%s' can be found.\r\n", name);
	send_to_char(my_buf, ch);
	return;
    }
    SAVE_CHAR_OBJ(savee, -20);
    sprintf(my_buf, "'%s' has been successfully saved.\r\n", name);
    send_to_char(my_buf, ch);
	
}
