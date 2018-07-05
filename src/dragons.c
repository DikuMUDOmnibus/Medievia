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
#include "dragon.h"

extern struct room_data *world[MAX_ROOM]; /* array of rooms                  */
extern struct char_data *character_list; /* global l-list of chars          */
extern char global_color;
extern struct descriptor_data *descriptor_list;
extern int dice(int number, int size);
extern struct char_data *mobs[MAX_MOB];
extern bool DigitString(char *szpText);
extern int number(int from, int to);
extern void remove_room_affect(struct room_affect *ra, char type);
extern void do_move(struct char_data *ch, char *argument, int cmd);
extern double dHoloDistance(int iX1,int iY1,int iX2,int iY2);
extern int choose_scent(struct char_data *ch, struct char_data *hunted);
extern void shout_hunting_message(struct char_data *ch);
extern struct zone_data *zone_table;
extern char track_colors[5][2];
extern char *track_messages[5][2][2];
extern ush_int Holo[MAXHOLO][MAXHOLO];
extern char MOUNTMOVE;
extern int iFlyStoreRoom;
extern int pulse;
extern int number_of_rooms;
extern struct HoloSurvey *survey_list;
extern struct HOLOROOMS *stpHoloRTemplates[256];



struct DRAGONSTRUCT gstaDragons[MAXDRAGONS];
int giNumPlayersInWilderness;
int giaDragonTypes[MAXDRAGONTYPES][2];
#define GOODDRAGON			0
#define EVILDRAGON			1

void RemoveDragonSurvey(int iSlot)
{
struct HoloSurvey *stpS,*stpSRemove;

	if(!gstaDragons[iSlot].stpSurvey)
		return;
	if(survey_list==gstaDragons[iSlot].stpSurvey){
		survey_list=gstaDragons[iSlot].stpSurvey->next;
		gstaDragons[iSlot].stpSurvey->description=my_free(gstaDragons[iSlot].stpSurvey->description);		
		gstaDragons[iSlot].stpSurvey=my_free(gstaDragons[iSlot].stpSurvey);
	}else{
		for(stpS=survey_list;stpS->next;stpS=stpS->next){
			if(stpS->next==gstaDragons[iSlot].stpSurvey)
				break;
		}
		if(stpS->next){
	   	 	stpSRemove=stpS->next;
			stpS->next=stpS->next->next;
			stpSRemove->description=my_free(stpSRemove->description);
    		stpSRemove=my_free(stpSRemove);   
			}
		}
	}

void do_CallDragon(struct char_data *stpCh, char *szpArg, int iCmd)
{
int iMob,x,y,xxx,yyy,iSlot;

	if(world[stpCh->in_room]->sector_type==SECT_INSIDE){
		global_color=32;
		send_to_char("You realize it is impossible to call a dragon from the indoors.\n\r",stpCh);
		global_color=0;
		return;
	}
	if(GET_LEVEL(stpCh)<13){
		global_color=32;
		send_to_char("You realize your not powerful enough to call a dragon.\n\r",stpCh);
		global_color=0;
	}
	global_color=32;
	act("$n uses $s powers to plee for a dragons aid.",TRUE,stpCh,0,0,TO_ROOM);
	send_to_char("You use you powers to plee for a dragons aid.",stpCh);
	global_color=0;
	for(x=0;x<MAXDRAGONTYPES;x++)
		if(!giaDragonTypes[x][GOODDRAGON])
			break;
	iMob=number(0,x-1);
	for(iSlot=0;iSlot<MAXDRAGONS;iSlot++)
		if(!gstaDragons[iSlot].stpDragon)
		break;
	if(iSlot==MAXDRAGONS){
		global_color=32;
		send_to_char("You get the feeling all dragons are busy.\n\r",stpCh);
		global_color=0;
		return;
	}
	gstaDragons[iSlot].stpDragon=read_mobile(giaDragonTypes[iMob][GOODDRAGON],REAL);
	SET_BIT(gstaDragons[iSlot].stpDragon->player.siMoreFlags,DRAGON);
	if(!gstaDragons[iSlot].stpDragon)
		SUICIDE;
	gstaDragons[iSlot].iPulse=pulse;
	gstaDragons[iSlot].iStatus=RESPONDING;
	gstaDragons[iSlot].iRoom=0;
	switch(number(0,3)){
		case 0:
			gstaDragons[iSlot].iX=HOLOX(stpCh)-SIGHTDIST+1;	
			gstaDragons[iSlot].iY=HOLOX(stpCh);
			break;
		case 1:
			gstaDragons[iSlot].iX=HOLOX(stpCh)+SIGHTDIST-1;	
			gstaDragons[iSlot].iY=HOLOX(stpCh);
			break;
		case 2:
			gstaDragons[iSlot].iX=HOLOX(stpCh);	
			gstaDragons[iSlot].iY=HOLOX(stpCh)-SIGHTDIST+1;
			break;
		case 3:
			gstaDragons[iSlot].iX=HOLOX(stpCh);	
			gstaDragons[iSlot].iY=HOLOX(stpCh)+SIGHTDIST-1;
			break;
	}
	if(gstaDragons[iSlot].iX<10)
		gstaDragons[iSlot].iX=10;
	if(gstaDragons[iSlot].iX>MAXHOLO-10)
		gstaDragons[iSlot].iX=MAXHOLO-10;
	if(gstaDragons[iSlot].iY<10)
		gstaDragons[iSlot].iY=10;
	if(gstaDragons[iSlot].iY>MAXHOLO-10)
		gstaDragons[iSlot].iY=MAXHOLO-10;
	
	gstaDragons[iSlot].stpCaller=stpCh;

	CREATE(gstaDragons[iSlot].stpSurvey,struct HoloSurvey,1);
	gstaDragons[iSlot].stpSurvey->iX=gstaDragons[iSlot].iX;
	gstaDragons[iSlot].stpSurvey->iY=gstaDragons[iSlot].iY;
	gstaDragons[iSlot].stpSurvey->dist=SIGHTDIST;
	CREATE(gstaDragons[iSlot].stpSurvey->description,char,200);
	sprintf(gstaDragons[iSlot].stpSurvey->description,"you see %s responding to someones plee",gstaDragons[iSlot].stpDragon->player.short_descr);
	gstaDragons[iSlot].stpSurvey->next=survey_list;
	survey_list=gstaDragons[iSlot].stpSurvey;

	x=gstaDragons[iSlot].iX-SIGHTDIST;
	if(x<1)x=1;
	y=gstaDragons[iSlot].iY-SIGHTDIST;
	if(y<1)y=1;
	xxx=gstaDragons[iSlot].iX+SIGHTDIST;
	if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
	yyy=gstaDragons[iSlot].iY+SIGHTDIST;
	if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
	HoloAct(x,y,gstaDragons[iSlot].iX,gstaDragons[iSlot].iY,xxx,yyy,
	"$n appear responding to someones plee",
	TRUE,gstaDragons[iSlot].stpDragon,NULL,NULL,33,"notice");
}

void MakeEvilDragon(void)
{
int iMob,x,y,xxx,yyy,iSlot,iStop;
struct descriptor_data *stpPeople;

	if(giNumPlayersInWilderness<1)
		return;
	iStop=number(0,giNumPlayersInWilderness);
	for(stpPeople=descriptor_list;iStop&&stpPeople;stpPeople=stpPeople->next){
		if(stpPeople->connected==CON_PLAYING
				&&world[stpPeople->character->in_room]->zone==197
				&&!stpPeople->character->specials.fighting
				&&GET_LEVEL(stpPeople->character)>12
				&&GET_LEVEL(stpPeople->character)<32)
			iStop--;		
	}
	if(iStop||!stpPeople)
		return;
	for(x=0;x<MAXDRAGONTYPES;x++)
		if(!giaDragonTypes[x][EVILDRAGON])
			break;
	iMob=number(0,x-1);
	for(iSlot=0;iSlot<MAXDRAGONS;iSlot++)
		if(!gstaDragons[iSlot].stpDragon)break;
	if(iSlot==MAXDRAGONS)
		SUICIDE;
	gstaDragons[iSlot].stpDragon=read_mobile(giaDragonTypes[iMob][EVILDRAGON],REAL);
	SET_BIT(gstaDragons[iSlot].stpDragon->player.siMoreFlags,DRAGON);
	if(!gstaDragons[iSlot].stpDragon)
		SUICIDE;
	gstaDragons[iSlot].iPulse=pulse;
	gstaDragons[iSlot].iStatus=CIRCLING;
	gstaDragons[iSlot].iRoom=0;
	gstaDragons[iSlot].iX=world[stpPeople->character->in_room]->holox;	
	gstaDragons[iSlot].iY=world[stpPeople->character->in_room]->holoy;
	gstaDragons[iSlot].stpCaller=NULL;
	CREATE(gstaDragons[iSlot].stpSurvey,struct HoloSurvey,1);
	gstaDragons[iSlot].stpSurvey->iX=gstaDragons[iSlot].iX;
	gstaDragons[iSlot].stpSurvey->iY=gstaDragons[iSlot].iY;
	gstaDragons[iSlot].stpSurvey->dist=SIGHTDIST;
	CREATE(gstaDragons[iSlot].stpSurvey->description,char,200);
	sprintf(gstaDragons[iSlot].stpSurvey->description,"you see %s circling...looking for prey!",gstaDragons[iSlot].stpDragon->player.short_descr);
	gstaDragons[iSlot].stpSurvey->next=survey_list;
	survey_list=gstaDragons[iSlot].stpSurvey;

	x=gstaDragons[iSlot].iX-SIGHTDIST;
	if(x<1)x=1;
	y=gstaDragons[iSlot].iY-SIGHTDIST;
	if(y<1)y=1;
	xxx=gstaDragons[iSlot].iX+SIGHTDIST;
	if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
	yyy=gstaDragons[iSlot].iY+SIGHTDIST;
	if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
	HoloAct(x,y,gstaDragons[iSlot].iX,gstaDragons[iSlot].iY,xxx,yyy,
	"$n Fly over and start circling ...looking for prey!",
	TRUE,gstaDragons[iSlot].stpDragon,NULL,NULL,33,"see");
}

struct room_affect *stpGetScent(int x, int y)
{
struct room_affect *stpRap;
struct descriptor_data *stpS;

	if(Holo[x][y]<256)
		return(NULL);
	for(stpRap=world[Holo[x][y]]->room_afs;stpRap;stpRap=stpRap->next){
		if(stpRap->type == RA_SCENT){
			for(stpS=descriptor_list;stpS;stpS=stpS->next){
				if(stpS->character&&stpS->character==stpRap->ch){
					if(GET_LEVEL(stpS->character)>12&&GET_LEVEL(stpS->character)<32)
						return(stpRap);
				}
			}
		}	
	}
	return(NULL);
}

		/* CALLED FROM EXTRACT_CHAR IF NOT CIRCLING*/
void RemoveDragon(struct char_data *stpDragon)
{
int iSlot;

	for(iSlot=0;iSlot<MAXDRAGONS;iSlot++){
		if(gstaDragons[iSlot].stpDragon==stpDragon){
			RemoveDragonSurvey(iSlot);
			gstaDragons[iSlot].stpDragon=NULL;			
			return;
		}
	}
}

void StartHunt(int iSlot, struct room_affect *stpRap)
{
int x,y,xxx,yyy;

	x=world[stpRap->room]->holox;
	y=world[stpRap->room]->holoy;
	gstaDragons[iSlot].iRoom=stpRap->room;
	gstaDragons[iSlot].iX=HOLORX(stpRap->room);
	gstaDragons[iSlot].iY=HOLORY(stpRap->room);
	gstaDragons[iSlot].iStatus=HUNTING;
	gstaDragons[iSlot].stpDragon->specials.hunting=stpRap->ch;
	char_to_room(gstaDragons[iSlot].stpDragon,stpRap->room);

	gstaDragons[iSlot].stpSurvey->iX=gstaDragons[iSlot].iX;
	gstaDragons[iSlot].stpSurvey->iY=gstaDragons[iSlot].iY;
	gstaDragons[iSlot].stpSurvey->dist=10;
	strcpy(gstaDragons[iSlot].stpSurvey->description,"you hear a dragon crashing through the area");

	x=gstaDragons[iSlot].iX-SIGHTDIST;
	if(x<1)x=1;
	y=gstaDragons[iSlot].iY-SIGHTDIST;
	if(y<1)y=1;
	xxx=gstaDragons[iSlot].iX+SIGHTDIST;
	if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
	yyy=gstaDragons[iSlot].iY+SIGHTDIST;
	if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
	HoloAct(x,y,gstaDragons[iSlot].iX,gstaDragons[iSlot].iY,xxx,yyy,
	"$n stop circling and land nearby...hunting someone!",
	TRUE,gstaDragons[iSlot].stpDragon,NULL,NULL,33,"notice");

}

void SearchForHunt(int iSlot)
{
int x,y,iDist,xx,yy;
struct room_affect *stpRap=NULL;
	
	x=gstaDragons[iSlot].iX;
	y=gstaDragons[iSlot].iY;
	iDist=2;

	/* Make larger and larger squares looking for prey, clockwise */
	if(!(stpRap=stpGetScent(x,y))){
		while(iDist<SIGHTDIST-1){
			x-=1;
			y-=1;
			iDist+=2;
			if(x<1||y<1||((x+iDist)>(MAXHOLO-2))||((y+iDist)>(MAXHOLO-2))){
				extract_char(gstaDragons[iSlot].stpDragon,TRUE);
				return;
			}
			yy=y;
			for(xx=x;xx<=x+iDist;xx++){
				stpRap=stpGetScent(xx,yy);
				if(stpRap){
					StartHunt(iSlot,stpRap);
					return;
				}
			}
			x=xx-1;
			for(yy=y;yy<=y+iDist;yy++){
				stpRap=stpGetScent(xx,yy);
				if(stpRap){
					StartHunt(iSlot,stpRap);
					return;
				}
			}
			y=yy-1;
			for(xx=x;xx>=x-iDist;xx--){
				stpRap=stpGetScent(xx,yy);
				if(stpRap){
					StartHunt(iSlot,stpRap);
					return;
				}
			}
			x=xx+1;
			for(yy=y;yy>=y-iDist;yy--){
				stpRap=stpGetScent(xx,yy);
				if(stpRap){
					StartHunt(iSlot,stpRap);
					return;
				}
			}
			y=yy+1;
		}
		/* could not find a scent trail for some reason */
		log_hd("### Dragon Code could not find a good scent trail to use");
		extract_char(gstaDragons[iSlot].stpDragon,TRUE);
		gstaDragons[iSlot].stpDragon=NULL;
		return;
	}else{
		StartHunt(iSlot,stpRap);
		return;
	}
}


void StartFight(int iSlot)
{
	global_color = track_colors[(int) GET_CLASS(gstaDragons[iSlot].stpDragon)][0];
	act( track_messages[(int) GET_CLASS(gstaDragons[iSlot].stpDragon)][0][0],FALSE, gstaDragons[iSlot].stpDragon,0,gstaDragons[iSlot].stpDragon->specials.hunting,TO_VICT);
	act( track_messages[(int) GET_CLASS(gstaDragons[iSlot].stpDragon)][0][1], FALSE, gstaDragons[iSlot].stpDragon, 0, gstaDragons[iSlot].stpDragon->specials.hunting, TO_NOTVICT);
	global_color=0;
	hit(gstaDragons[iSlot].stpDragon,gstaDragons[iSlot].stpDragon->specials.hunting,0);
	gstaDragons[iSlot].iStatus=FIGHTING;
	RemoveDragonSurvey(iSlot);
}

void Hunt(int iSlot)
{
int door,x,y,xxx,yyy,room;
struct room_affect *rap;

	if(gstaDragons[iSlot].stpDragon->specials.fighting){
		if(gstaDragons[iSlot].stpDragon->specials.fighting->in_room==gstaDragons[iSlot].stpDragon->in_room){
			gstaDragons[iSlot].iStatus=FIGHTING;
			return;
		}
		if(IS_SET(gstaDragons[iSlot].stpDragon->specials.fighting->player.siMoreFlags,DRAGON))
			return;
	}
	if((pulse-gstaDragons[iSlot].iPulse)>PULSEBEFORESTOPHUNT){
		x=gstaDragons[iSlot].iX-SIGHTDIST;
		if(x<1)x=1;
		y=gstaDragons[iSlot].iY-SIGHTDIST;
		if(y<1)y=1;
		xxx=gstaDragons[iSlot].iX+SIGHTDIST;
		if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
		yyy=gstaDragons[iSlot].iY+SIGHTDIST;
		if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
		HoloAct(x,y,gstaDragons[iSlot].iX,gstaDragons[iSlot].iY,xxx,yyy,
		"$n ROAR as $e takes off and flies away...giving up his hunt.",
		TRUE,gstaDragons[iSlot].stpDragon,NULL,NULL,33,"notice");
		extract_char(gstaDragons[iSlot].stpDragon,TRUE);
		return;
	}
	door=choose_scent(gstaDragons[iSlot].stpDragon, gstaDragons[iSlot].stpDragon->specials.hunting);
	if(number(1,25)==7 && door != 6 && door !=7)
		shout_hunting_message(gstaDragons[iSlot].stpDragon);
	if(door == 6){
		x=gstaDragons[iSlot].iX-SIGHTDIST;
		if(x<1)x=1;
		y=gstaDragons[iSlot].iY-SIGHTDIST;
		if(y<1)y=1;
		xxx=gstaDragons[iSlot].iX+SIGHTDIST;
		if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
		yyy=gstaDragons[iSlot].iY+SIGHTDIST;
		if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
		HoloAct(x,y,gstaDragons[iSlot].iX,gstaDragons[iSlot].iY,xxx,yyy,
		"$n ROAR as $e takes off and flies away...giving up his hunt, $e must have lost the scent trail.",
		TRUE,gstaDragons[iSlot].stpDragon,NULL,NULL,33,"notice");
		extract_char(gstaDragons[iSlot].stpDragon,TRUE);
		return;
	}
	if(door==7){
		StartFight(iSlot);
		return;
	}
	if(door<=5&&world[gstaDragons[iSlot].stpDragon->in_room]->dir_option[door]){
		room=EXIT(gstaDragons[iSlot].stpDragon,door)->to_room;
		if(!world[room]){
			extract_char(gstaDragons[iSlot].stpDragon,TRUE);	
			return;
		}
		if(GET_POS(gstaDragons[iSlot].stpDragon) == POSITION_STANDING)
		if(CAN_GO(gstaDragons[iSlot].stpDragon, door))
		if(!IS_SET(world[room]->room_flags, NO_MOB|DEATH)){
			for(rap=world[room]->room_afs;rap;rap=rap->next){
				if(rap->type==RA_SHIELD){
					global_color=33;
					send_to_room("The area's shield HUMS and FLASHES as a Dragon BANGS into it close by.\n\r",rap->room);
					remove_room_affect(rap,0);
					global_color=0;
				}
			}
			gstaDragons[iSlot].stpDragon->specials.last_direction = door;
			do_move( gstaDragons[iSlot].stpDragon, "", ++door );
			gstaDragons[iSlot].iX=HOLOX(gstaDragons[iSlot].stpDragon);
			gstaDragons[iSlot].iY=HOLOY(gstaDragons[iSlot].stpDragon);
			gstaDragons[iSlot].iRoom=gstaDragons[iSlot].stpDragon->in_room;
			if(gstaDragons[iSlot].stpSurvey){
				gstaDragons[iSlot].stpSurvey->iX=gstaDragons[iSlot].iX;
				gstaDragons[iSlot].stpSurvey->iY=gstaDragons[iSlot].iY;
			}
		}
		return;
	}
	extract_char(gstaDragons[iSlot].stpDragon,TRUE);	
}

void Fight(int iSlot)
{
int x,y,xxx,yyy;
struct char_data *stpMob,*stpMobNext;

	if(!gstaDragons[iSlot].stpDragon->specials.fighting){
		gstaDragons[iSlot].iStatus=HUNTING;
		return;
	}
	if(gstaDragons[iSlot].stpDragon->specials.hunting!=gstaDragons[iSlot].stpDragon->specials.fighting)
		return;
	if(gstaDragons[iSlot].stpDragon->specials.fighting->in_room!=gstaDragons[iSlot].stpDragon->in_room){
		gstaDragons[iSlot].iStatus=HUNTING;
		return;
	}
	for(stpMob=world[gstaDragons[iSlot].stpDragon->in_room]->people;stpMob;stpMob=stpMobNext){
		stpMobNext=stpMob->next_in_room;
		if(!stpMob->desc&&!IS_SET(stpMob->player.siMoreFlags,DRAGON)){
			act("$n slays $N!",TRUE,gstaDragons[iSlot].stpDragon,0,stpMob,TO_ROOM);
			raw_kill(stpMob,NULL);
			break;
		}
	}
	if(GET_HIT(gstaDragons[iSlot].stpDragon->specials.fighting)<(int)(GET_MAX_HIT(gstaDragons[iSlot].stpDragon->specials.fighting)/20)){
		GET_GOLD(gstaDragons[iSlot].stpDragon->specials.fighting)=0;
		global_color=33;
		act("$n PINS you down!  You put all of your gold into a sack on its neck.",TRUE,gstaDragons[iSlot].stpDragon,0,0,TO_CHAR);
		act("$n PINS $N down!  $N puts all $S gold into a sack on its neck.",TRUE,gstaDragons[iSlot].stpDragon,0,gstaDragons[iSlot].stpDragon->specials.fighting,TO_ROOM);
		global_color=0;

		x=gstaDragons[iSlot].iX-SIGHTDIST;
		if(x<1)x=1;
		y=gstaDragons[iSlot].iY-SIGHTDIST;
		if(y<1)y=1;
		xxx=gstaDragons[iSlot].iX+SIGHTDIST;
		if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
		yyy=gstaDragons[iSlot].iY+SIGHTDIST;
		if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
		HoloAct(x,y,gstaDragons[iSlot].iX,gstaDragons[iSlot].iY,xxx,yyy,
		"$n bolt into the air and fly off..$s sack filled with gold",
		TRUE,gstaDragons[iSlot].stpDragon,NULL,NULL,33,"notice");
		GET_POS(gstaDragons[iSlot].stpDragon->specials.fighting)=POSITION_RESTING;
		stop_fighting(gstaDragons[iSlot].stpDragon);
		extract_char(gstaDragons[iSlot].stpDragon,TRUE);
	}	
}

void Defend(int iSlot)
{
int x,y,xxx,yyy;

	if(!gstaDragons[iSlot].stpDragon->specials.fighting){
		x=gstaDragons[iSlot].iX-SIGHTDIST;
		if(x<1)x=1;
		y=gstaDragons[iSlot].iY-SIGHTDIST;
		if(y<1)y=1;
		xxx=gstaDragons[iSlot].iX+SIGHTDIST;
		if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
		yyy=gstaDragons[iSlot].iY+SIGHTDIST;
		if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
		HoloAct(x,y,gstaDragons[iSlot].iX,gstaDragons[iSlot].iY,xxx,yyy,
		"$n bolt into the air and fly off...$e is carrying an evil dragon head in $e claws",
		TRUE,gstaDragons[iSlot].stpDragon,NULL,NULL,33,"notice");
		extract_char(gstaDragons[iSlot].stpDragon,TRUE);
	}
}

void Respond(int iSlot)
{
struct descriptor_data *stpS;
int iE,iW,iN,iS,x=0,y=0,xxx,yyy,i;

	for(stpS=descriptor_list;stpS;stpS=stpS->next){
		if(stpS->character==gstaDragons[iSlot].stpCaller){
			if(dHoloDistance(HOLOX(gstaDragons[iSlot].stpCaller),HOLOY(gstaDragons[iSlot].stpCaller),gstaDragons[iSlot].iX,gstaDragons[iSlot].iY)<6){
				for(x=0;x<MAXDRAGONS;x++){
					if(gstaDragons[x].stpDragon&&gstaDragons[x].stpDragon->specials.hunting==gstaDragons[iSlot].stpCaller){
						/* this guy is being hunted */
						gstaDragons[iSlot].iStatus=DEFENDING;
						char_to_room(gstaDragons[iSlot].stpDragon,gstaDragons[x].stpDragon->in_room);
						i=x;
						gstaDragons[iSlot].iX=HOLOX(gstaDragons[x].stpDragon);
						gstaDragons[iSlot].iY=HOLOY(gstaDragons[x].stpDragon);
						gstaDragons[iSlot].iRoom=gstaDragons[x].stpDragon->in_room;
						RemoveDragonSurvey(x);
						strcpy(gstaDragons[iSlot].stpSurvey->description,"you hear two dragons in mortal combat");
						x=gstaDragons[iSlot].iX-SIGHTDIST;
						if(x<1)x=1;
						y=gstaDragons[iSlot].iY-SIGHTDIST;
						if(y<1)y=1;
						xxx=gstaDragons[iSlot].iX+SIGHTDIST;
						if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
						yyy=gstaDragons[iSlot].iY+SIGHTDIST;
						if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
						HoloAct(x,y,gstaDragons[iSlot].iX,gstaDragons[iSlot].iY,xxx,yyy,
						"$n dive down and you hear $m pounce on another dragon.",
						TRUE,gstaDragons[iSlot].stpDragon,NULL,NULL,31,"see");
						hit(gstaDragons[iSlot].stpDragon,gstaDragons[i].stpDragon,0);
						return;
					}
				}
				gstaDragons[iSlot].iStatus=FLYING;
				char_to_room(gstaDragons[iSlot].stpDragon,stpS->character->in_room);
				gstaDragons[iSlot].iX=HOLOX(gstaDragons[x].stpDragon);
				gstaDragons[iSlot].iY=HOLOY(gstaDragons[x].stpDragon);
				gstaDragons[iSlot].iRoom=gstaDragons[x].stpDragon->in_room;
				RemoveDragonSurvey(iSlot);
				act("A wise looking dragon crashes into the room, ready to be mounted.",TRUE,gstaDragons[iSlot].stpDragon,0,0,TO_ROOM);
				return;
			}
			iE=gstaDragons[iSlot].iX-HOLOX(stpS->character);
			iW=HOLOX(stpS->character)-gstaDragons[iSlot].iX;
			iS=gstaDragons[iSlot].iY-HOLOY(stpS->character);
			iN=HOLOY(stpS->character)-gstaDragons[iSlot].iY;
			if(iE>=iW&&iE>=iS&&iE>=iN)
				x=-5;
			if(iW>=iE&&iW>=iS&&iW>=iN)
				x=5;
			if(iS>=iE&&iS>=iW&&iS>=iN)
				y=-5;
			if(iN>=iE&&iN>=iW&&iN>=iS)
				y=5;
			gstaDragons[iSlot].iX+=x;
			gstaDragons[iSlot].iY+=y;
			gstaDragons[iSlot].stpSurvey->iX=gstaDragons[iSlot].iX;
			gstaDragons[iSlot].stpSurvey->iY=gstaDragons[iSlot].iY;
			return;
		}
	}
	extract_char(gstaDragons[iSlot].stpDragon,TRUE);	
}

void Flying(int iSlot)
{
int x,y,xxx,yyy;

	if(gstaDragons[iSlot].stpDragon->in_room!=iFlyStoreRoom){
		if((pulse-gstaDragons[iSlot].iPulse)>FLYTIMEPULSE){
			x=HOLOX(gstaDragons[iSlot].stpDragon)-SIGHTDIST;
			if(x<1)x=1;
			y=HOLOY(gstaDragons[iSlot].stpDragon)-SIGHTDIST;
			if(y<1)y=1;
			xxx=HOLOX(gstaDragons[iSlot].stpDragon)+SIGHTDIST;
			if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
			yyy=HOLOY(gstaDragons[iSlot].stpDragon)+SIGHTDIST;
			if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
			HoloAct(x,y,HOLOX(gstaDragons[iSlot].stpDragon),HOLOY(gstaDragons[iSlot].stpDragon),xxx,yyy,
			"$n fly up and away slowly..",
			TRUE,gstaDragons[iSlot].stpDragon,NULL,NULL,31,"notice");
			extract_char(gstaDragons[iSlot].stpDragon,TRUE);
			return;
		}
		if(!gstaDragons[iSlot].stpDragon->specials.stpMount){
			if(number(0,100)<10){
				global_color=34;
				act("$n looks around...waiting to be mounted..looks as if $e may just fly off.",TRUE,gstaDragons[iSlot].stpDragon,0,0,TO_ROOM);
				global_color=0;
			}
			return;
		}
		if(number(0,100)<10){
			global_color=34;
			act("$n stretches $s wings...wanting to fly.",TRUE,gstaDragons[iSlot].stpDragon,0,0,TO_ROOM);
			global_color=0;
		}
	}
}

void DragonControl(void)
{
int iNumDragons=0,x;
struct descriptor_data *stpPeople;

	if((pulse %  (4*60)) == 0){
		giNumPlayersInWilderness=0;
		for(stpPeople=descriptor_list;stpPeople;stpPeople=stpPeople->next){
			if(stpPeople->connected==CON_PLAYING
				&&world[stpPeople->character->in_room]->zone==197
				&&!stpPeople->character->specials.fighting
				&&GET_LEVEL(stpPeople->character)>12)
				giNumPlayersInWilderness++;
		}
		for(x=0;x<MAXDRAGONS;x++)
			if(gstaDragons[x].stpDragon)
				iNumDragons++;

		if(iNumDragons<MAXDRAGONS/2){/* need more dragons? */
			if(iNumDragons<(giNumPlayersInWilderness/PEOPLEPERDRAGON)){
				MakeEvilDragon();
			}
		}
	}

	for(x=0;x<MAXDRAGONS;x++){
		if(gstaDragons[x].stpDragon){
			switch(gstaDragons[x].iStatus){
				case CIRCLING:
					if((pulse-gstaDragons[x].iPulse)>PULSEBEFOREHUNT){
						SearchForHunt(x);
					}
					break;
				case HUNTING:
					Hunt(x);
					break;
				case FIGHTING:
					Fight(x);
					break;
				case RESPONDING:
					Respond(x);
					break;
				case DEFENDING:
					Defend(x);
					break;
				case FLYING:
					Flying(x);
					break;
				default:
					SUICIDE;
			}
		}
	}
}

void LoadDragonList(void)
{
FILE *fl;
char szTag[8192];
int x,iSlot,iType,iMob;

    if(!(fl=fopen("../lib/medievia.dragons", "r"))){
		perror("Opening medievia.dragons");
		SUICIDE;
   	}
	for(x=0;x<MAXDRAGONTYPES;x++){
		giaDragonTypes[x][GOODDRAGON]=0;
		giaDragonTypes[x][EVILDRAGON]=0;
	}	
	iSlot=0;
    fprintf(stderr,"DRAGONS:  Loading Dragon Data...\n");
	iType=GOODDRAGON;
	while(1){
		fscanf(fl," %s ",szTag);
		if(szTag[0]=='$')
			break;
		if(!str_cmp(szTag,"GOODDRAGONS")){
			iType=GOODDRAGON;
			iSlot=0;
		}else if(!str_cmp(szTag,"EVILDRAGONS")){
			iType=EVILDRAGON;
			iSlot=0;
		}else if(DigitString(szTag)){
			iMob=atoi(szTag);
			if(iMob<0||iMob>MAX_MOB)
				SUICIDE;
			if(!mobs[iMob])
				SUICIDE;
			if(iType==MAXDRAGONTYPES)
				SUICIDE;
			giaDragonTypes[iSlot][iType++]=iMob;
		}else{
			SUICIDE;
		}
	}
	fclose(fl);
}

void SetupDragons(void)
{
int x;

	fprintf(stderr,"DRAGONS: Initializing Dragon Control Center...\n");
	for(x=0;x<MAXDRAGONS;x++)
		gstaDragons[x].stpDragon=NULL;
	LoadDragonList();
}

