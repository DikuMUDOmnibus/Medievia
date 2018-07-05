/***************************************************************************
*					 MEDIEVIA CyberSpace Code and Data files		  	   *
*       Copyright (C) 1991, 1996 INTENSE Software(tm) and Mike Krause	   *
*							   All rights reserved				           *
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
#include <unistd.h>
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "spells.h"
#include "holocode.h"

extern struct room_data *world[MAX_ROOM]; /* array of rooms                  */
extern struct char_data *character_list; /* global l-list of chars          */
extern char global_color;
extern struct descriptor_data *descriptor_list;
extern int dice(int number, int size);
extern void die_formation(struct char_data *ch);
extern bool is_formed(struct char_data *ch);
extern int number(int from, int to);
extern int iMakeHoloRoom(int x,int y);
extern ush_int Holo[MAXHOLO][MAXHOLO];

void do_mount(struct char_data *stpCh, char *szpArg, int iCmd)
{
struct char_data *stpMob;
char szBuf[MAX_INPUT_LENGTH];

	if(IS_UNDEAD(stpCh)){
		send_to_char("You don't even try as it is scared of you.\n\r",stpCh);
		return;
	}
	if(IS_NPC(stpCh))
		return;
	if(stpCh->specials.stpMount){
		sprintf(szBuf,"You realize you are already mounted on %s\n\r",stpCh->specials.stpMount->player.short_descr);
		global_color=33;
		send_to_char(szBuf,stpCh);
		global_color=0;
		return;
	}
	one_argument(szpArg,szBuf);
	if(!szBuf[0]){
		send_to_char("Mount what?\n\r",stpCh);
		return;
	}
	for(stpMob=world[stpCh->in_room]->people;stpMob;stpMob=stpMob->next_in_room){
		if(!IS_SET(stpMob->player.siMoreFlags,MOUNT))
			continue;
		if(!IS_NPC(stpMob))
			continue;
		if(stpMob->specials.stpMount)
			continue;
		if(!isname(szBuf,stpMob->player.name))
			continue;
		break;
	}
	if(!stpMob){
		global_color=33;
		send_to_char("You realize nothing like that is here that is mountable right now.\n\r",stpCh);
		global_color=0;
		return;
	}
 	if(IS_GOOD(stpMob)&&!IS_GOOD(stpCh)){
	  act("$n tries to mount $N, but is thrown off.",TRUE,stpCh,0,stpMob,TO_ROOM);
	  act("You try to mount $N, but $E throws you off.",TRUE,stpCh,0,stpMob,TO_CHAR);
	  return;
	}
 	if(IS_EVIL(stpMob)&&!IS_EVIL(stpCh)){
	  act("$n tries to mount $N, but is thrown off.",TRUE,stpCh,0,stpMob,TO_ROOM);
	  act("You try to mount $N, but $E throws you off.",TRUE,stpCh,0,stpMob,TO_CHAR);
	  return;
	}
	if(is_formed(stpMob))
		die_formation(stpMob);
	stpMob->specials.stpMount=stpCh;
	stpCh->specials.stpMount=stpMob;
	global_color=32;
	act("$n jumps up and mounts $N.",TRUE,stpCh,0,stpMob,TO_ROOM);
	act("You jump up and mount $N.",TRUE,stpCh,0,stpMob,TO_CHAR);
	global_color=0;
    if(IS_AFFECTED(stpCh, AFF_SNEAK))
        affect_from_char(stpCh, SKILL_SNEAK);
}

void do_unmount(struct char_data *stpCh, char *szpArg, int iCmd)
{

	if(IS_NPC(stpCh))
		return;
	if(!stpCh->specials.stpMount){
		global_color=33;
		send_to_char("You laugh and realize you are not mounted on any beast.\n\r",stpCh);
		global_color=0;
		return;
	}
	if(stpCh->specials.stpMount->specials.stpMount!=stpCh)
		SUICIDE;
	global_color=32;
	act("$n jumps down and unmounts $N.",TRUE,stpCh,0,stpCh->specials.stpMount,TO_ROOM);
	act("You jump down and unmount $N.",TRUE,stpCh,0,stpCh->specials.stpMount,TO_CHAR);
	global_color=0;
	stpCh->specials.stpMount->specials.stpMount=NULL;
	stpCh->specials.stpMount=NULL;
}
