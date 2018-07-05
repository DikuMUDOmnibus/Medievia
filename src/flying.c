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
extern int number(int from, int to);
extern int iMakeHoloRoom(int x,int y);
extern struct zone_data *zone_table;
extern ush_int Holo[MAXHOLO][MAXHOLO];
extern char MOUNTMOVE;
extern int pulse;
extern int number_of_rooms;
extern struct HoloSurvey *survey_list;
extern struct HOLOROOMS *stpHoloRTemplates[256];
extern void do_areamap(struct char_data *stpCh, char *szpArgument, int iCmd);
extern void do_land(struct char_data *stpCh, char *szpArgument, int iCmd);

void Flyup(struct char_data *stpCh);
void Flydown(struct char_data *stpCh);
void ForceLanding(struct char_data *stpCh);

int iFlyStoreRoom;

#define SIGHTDIST 40

void do_fly(struct char_data *stpCh, char *szpArgument, int iCmd)
{
char szBuf[MAX_INPUT_LENGTH*2];
int x,y,xxx,yyy;

	if(IS_NPC(stpCh))
		return;
	if(!stpCh->specials.stpMount){
		global_color=31;
		send_to_char("You chuckle as you realize your not mounted on any animal.\n\r",stpCh);
		global_color=0;
		return;
	}	
	if(!IS_SET(stpCh->specials.stpMount->player.siMoreFlags,FLY)){
		global_color=31;
		send_to_char("You chuckle as you realize your not mounted on a beast that can fly.\n\r",stpCh);
		global_color=0;
		return;
	}
	if(!IS_FLYING(stpCh)&&world[stpCh->in_room]->sector_type == SECT_INSIDE){
		global_color=31;
		send_to_char("You think you better not fly indoors!.\n\r",stpCh);
		global_color=0;
		return;
	}
	if(!IS_FLYING(stpCh)&&GET_MOVE(stpCh->specials.stpMount)<20){
		send_to_char("You see that your mount cant seem to fly yet..\n\r",stpCh);
		return;
	}
	if(stpCh->specials.fighting){
		global_color=31;
		send_to_char("Your mount would never leave a fight!\\r",stpCh);
		global_color=0;
		return;
	}
	if(!IS_FLYING(stpCh)){
		Flyup(stpCh);
		return;
	}
	one_argument(szpArgument,szBuf);
	switch(szBuf[0]){
		case 'N':
		case 'n':
			stpCh->p->stpFlying->iDir=NORTH;
			strcpy(szBuf,"$N carrying $n bank..changing directions and fly Northbound.");
			sprintf(stpCh->p->stpFlying->stpSurvey->description,"you see %s carrying %s flying Northbound",stpCh->specials.stpMount->player.short_descr,GET_NAME(stpCh));
			break;
		case 'S':
		case 's':
			stpCh->p->stpFlying->iDir=SOUTH;
			strcpy(szBuf,"$N carrying $n bank..changing directions and fly Souththbound.");
			sprintf(stpCh->p->stpFlying->stpSurvey->description,"you see %s carrying %s flying Southbound",stpCh->specials.stpMount->player.short_descr,GET_NAME(stpCh));
			break;
		case 'E':
		case 'e':
			stpCh->p->stpFlying->iDir=EAST;
			strcpy(szBuf,"$N carrying $n bank..changing directions and fly Eastbound.");
			sprintf(stpCh->p->stpFlying->stpSurvey->description,"you see %s carrying %s flying Eastbound",stpCh->specials.stpMount->player.short_descr,GET_NAME(stpCh));
			break;
		case 'W':
		case 'w':
			stpCh->p->stpFlying->iDir=WEST;
			strcpy(szBuf,"$N carrying $n bank..changing directions and fly Westbound.");
			sprintf(stpCh->p->stpFlying->stpSurvey->description,"you see %s carrying %s flying Westbound",stpCh->specials.stpMount->player.short_descr,GET_NAME(stpCh));
			break;
		case 0:
			stpCh->p->stpFlying->iDir=-1;
			strcpy(szBuf,"$N carrying $n stop flying and circle slowly.");
			sprintf(stpCh->p->stpFlying->stpSurvey->description,"you see %s carrying %s circling slowly",stpCh->specials.stpMount->player.short_descr,GET_NAME(stpCh));
			break;
		
		default:
			send_to_char("You can only fly North South East or West.\n\r",stpCh);
			return;
			break;
	}
	global_color=32;
	if(stpCh->p->stpFlying->iDir==-1)
		act("You hold on tighter and start circling slowly.",TRUE,stpCh,0,0,TO_CHAR);
	else
		act("You lean out that way and $N changes direction.",TRUE,stpCh,NULL,stpCh->specials.stpMount,TO_CHAR);
	global_color=0;
	x=stpCh->p->stpFlying->iX-SIGHTDIST;
	if(x<1)x=1;
	y=stpCh->p->stpFlying->iY-SIGHTDIST;
	if(y<1)y=1;
	xxx=stpCh->p->stpFlying->iX+SIGHTDIST;
	if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
	yyy=stpCh->p->stpFlying->iY+SIGHTDIST;
	if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
	HoloAct(x,y,stpCh->p->stpFlying->iX,stpCh->p->stpFlying->iY,xxx,yyy,szBuf,TRUE,stpCh,NULL,stpCh->specials.stpMount,32,"notice");	
}

void MoveFlyingPeople(void)
{
struct char_data *stpV,*stpVN;
int x,y,tox,toy;
int ex,ey,exx,eyy,lx,ly,lxx,lyy;
char szEBuf[200],szLBuf[200];

	for(stpV=world[iFlyStoreRoom]->people;stpV;stpV=stpVN){
		stpVN=stpV->next_in_room;
		if(IS_NPC(stpV)||!IS_FLYING(stpV))
			continue;
		if(GET_MOVE(stpV->specials.stpMount)<6){
			ForceLanding(stpV);
			return;
		}
		if(stpV->p->stpFlying->iDir==-1){
			global_color=32;
			send_to_char("Your mount works hard flying in small circles.\n\r",stpV);			
			global_color=0;
			GET_MOVE(stpV->specials.stpMount)-=10;
			if(GET_MOVE(stpV->specials.stpMount)<0)
				GET_MOVE(stpV->specials.stpMount)=0;
		}else{
			x=stpV->p->stpFlying->iX;					
			y=stpV->p->stpFlying->iY;					
			switch(stpV->p->stpFlying->iDir){
				case NORTH:
					tox=x;
					toy=y-10;
					if(y<10){
						y=10;
						stpV->p->stpFlying->iDir=-1;
						global_color=33;
						send_to_char("You hit the northern edge of Vryce protected land. You circle in place and decide which way to fly.\n\r",stpV);
						global_color=0;
					}
					ex=x-SIGHTDIST;
					ey=y-SIGHTDIST;
					exx=x+SIGHTDIST;
					eyy=y-SIGHTDIST+10;
					strcpy(szEBuf,"$N carrying $n come into view flying Northbound.");
					lx=x-SIGHTDIST;
					ly=y+SIGHTDIST;
					lxx=x+SIGHTDIST;
					lyy=y+SIGHTDIST+10;
					strcpy(szLBuf,"$N carrying $n go over the horizon flying Northbound.");
					break;
				case SOUTH:
					tox=x;
					toy=y+10;
					if(y>(MAXHOLO-10)){
						y=MAXHOLO-10;
						stpV->p->stpFlying->iDir=-1;
						global_color=33;
						send_to_char("You hit the southern edge of Vryce protected land. You circle in place and decide which way to fly.\n\r",stpV);
						global_color=0;
					}
					ex=x-SIGHTDIST;
					ey=y+SIGHTDIST-10;
					exx=x+SIGHTDIST;
					eyy=y+SIGHTDIST;
					strcpy(szEBuf,"$N carrying $n come into view flying Southbound.");
					lx=x-SIGHTDIST;
					ly=y-SIGHTDIST-10;
					lxx=x+SIGHTDIST;
					lyy=y-SIGHTDIST;
					strcpy(szLBuf,"$N carrying $n go over the horizon flying Southbound.");
					break;
				case EAST:
					tox=x+10;
					toy=y;
					if(x>(MAXHOLO-10)){
						x=MAXHOLO-10;
						stpV->p->stpFlying->iDir=-1;
						global_color=33;
						send_to_char("You hit the eastern edge of Vryce protected land. You circle in place and decide which way to fly.\n\r",stpV);
						global_color=0;
					}
					ex=x+SIGHTDIST-10;
					ey=y-SIGHTDIST;
					exx=x+SIGHTDIST;
					eyy=y+SIGHTDIST;
					strcpy(szEBuf,"$N carrying $n come into view flying Eastbound.");
					lx=x-SIGHTDIST-10;
					ly=y-SIGHTDIST;
					lxx=x-SIGHTDIST;
					lyy=y+SIGHTDIST;
					strcpy(szLBuf,"$N carrying $n go over the horizon flying Eastbound.");
					break;
				case WEST:
					tox=x-10;
					toy=y;
					if(x<10){
						x=10;
						stpV->p->stpFlying->iDir=-1;
						global_color=33;
						send_to_char("You hit the western edge of Vryce protected land. You circle in place and decide which way to fly.\n\r",stpV);
						global_color=0;
					}
					ex=x-SIGHTDIST;
					ey=y-SIGHTDIST;
					exx=x-SIGHTDIST+10;
					eyy=y+SIGHTDIST;
					strcpy(szEBuf,"$N carrying $n come into view flying Westbound.");
					lx=x+SIGHTDIST;
					ly=y-SIGHTDIST;
					lxx=x+SIGHTDIST+10;
					lyy=y+SIGHTDIST;
					strcpy(szLBuf,"$N carrying $n go over the horizon flying Westbound.");
					break;
				default:
					SUICIDE;
					return;
			
			}
			GET_MOVE(stpV->specials.stpMount)-=10;
			if(GET_MOVE(stpV->specials.stpMount)<0)
				GET_MOVE(stpV->specials.stpMount)=0;
			stpV->p->stpFlying->iX=tox;						
			stpV->p->stpFlying->iY=toy;
			stpV->p->stpFlying->stpSurvey->iX=tox;
			stpV->p->stpFlying->stpSurvey->iY=toy;
			do_areamap(stpV,"",9);
			HoloAct(ex,ey,tox,toy,exx,eyy,szEBuf,TRUE,stpV,NULL,stpV->specials.stpMount,32,"see");			
			HoloAct(lx,ly,tox,toy,lxx,lyy,szLBuf,TRUE,stpV,NULL,stpV->specials.stpMount,32,"see");			
		}
	}

}

void ForceLanding(struct char_data *stpCh)
{

	global_color=31;	
	send_to_char("Your mount, out of strength, heads for a place to land..\n\r",stpCh);
	do_land(stpCh,"",9);
	if(IS_FLYING(stpCh)){
		if(stpCh->p->stpFlying->iX>(MAXHOLO/2))
			stpCh->p->stpFlying->iX-=number(1,10);			
		else
			stpCh->p->stpFlying->iX+=number(1,10);			
		if(stpCh->p->stpFlying->iY>(MAXHOLO/2))
			stpCh->p->stpFlying->iY-=number(1,10);	
		else
			stpCh->p->stpFlying->iY+=number(1,10);	
		do_areamap(stpCh,"",9);
	}
}

void Flyup(struct char_data *stpCh)
{
struct FLYING *stpF;
int x,y,xxx,yyy;

	global_color=34;
	act("You hang on tighter...click your heels on $S sides..",FALSE,stpCh,0,stpCh->specials.stpMount,TO_CHAR);
	act("$N flaps $S powerful wings and you shoot upward..soaring in small circles.",FALSE,stpCh,0,stpCh->specials.stpMount,TO_CHAR);
	global_color=31;
	act("$n hangs on tighter...$e click $s heels on $S sides..",FALSE,stpCh,0,stpCh->specials.stpMount,TO_ROOM);
	global_color=0;	
	SET_BIT(stpCh->player.siMoreFlags,FLY);
	CREATE(stpF, struct FLYING, 1);
	stpF->iX=HOLOX(stpCh);
	stpF->iY=HOLOY(stpCh);
	stpF->iDir=-1;
	stpCh->p->stpFlying=stpF;
	CREATE(stpF->stpSurvey,struct HoloSurvey,1);
	stpF->stpSurvey->iX=stpF->iX;
	stpF->stpSurvey->iY=stpF->iY;
	stpF->stpSurvey->dist=SIGHTDIST;
	CREATE(stpF->stpSurvey->description,char,200);
	sprintf(stpF->stpSurvey->description,"you see %s carrying %s flying in circles",stpCh->specials.stpMount->player.short_descr,GET_NAME(stpCh));
	stpF->stpSurvey->next=survey_list;
	survey_list=stpF->stpSurvey;
	MOUNTMOVE=TRUE;
	char_from_room(stpCh);
	char_to_room(stpCh,iFlyStoreRoom);
	MOUNTMOVE=FALSE;

	x=stpF->iX-SIGHTDIST;
	if(x<1)x=1;
	y=stpF->iY-SIGHTDIST;
	if(y<1)y=1;
	xxx=stpF->iX+SIGHTDIST;
	if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
	yyy=stpF->iY+SIGHTDIST;
	if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
	HoloAct(x,y,stpF->iX,stpF->iY,xxx,yyy,
	"$N carrying $n fly up into the sky and soar in small circles.",
	TRUE,stpCh,NULL,stpCh->specials.stpMount,33,"see");
}

void do_land(struct char_data *stpCh, char *szpArgument, int iCmd)
{
char OK=TRUE;

	if(IS_NPC(stpCh))
		return;
	if(!stpCh->specials.stpMount){
		global_color=31;
		send_to_char("You chuckle as you realize your not mounted on any animal.\n\r",stpCh);
		global_color=0;
		return;
	}	
	if(!IS_SET(stpCh->specials.stpMount->player.siMoreFlags,FLY)){
		global_color=31;
		send_to_char("You chuckle as you realize your not mounted on a beast that can fly.\n\r",stpCh);
		global_color=0;
		return;
	}
	if(!IS_FLYING(stpCh)){
		global_color=31;
		send_to_char("You chuckle as you realize your not flying!.\n\r",stpCh);
		global_color=0;
		return;
	}
	if(Holo[stpCh->p->stpFlying->iX][stpCh->p->stpFlying->iY]<256){
		if(IS_SET(stpHoloRTemplates[Holo[stpCh->p->stpFlying->iX][stpCh->p->stpFlying->iY]]->room_flags,BLOCKING))
			OK=FALSE;	
		if(IS_SET(stpHoloRTemplates[Holo[stpCh->p->stpFlying->iX][stpCh->p->stpFlying->iY]]->extra_flags,TRADE_NO_HORSE))
			OK=FALSE;	
	}else{
		if(IS_SET(world[Holo[stpCh->p->stpFlying->iX][stpCh->p->stpFlying->iY]]->room_flags,BLOCKING))
			OK=FALSE;
		if(IS_SET(world[Holo[stpCh->p->stpFlying->iX][stpCh->p->stpFlying->iY]]->extra_flags,TRADE_NO_HORSE))
			OK=FALSE;
		if(world[Holo[stpCh->p->stpFlying->iX][stpCh->p->stpFlying->iY]]->sector_type == SECT_INSIDE)
			OK=FALSE;
	}
	if(!OK){	
		global_color=31;
		send_to_char("You take a closer look and see that this is not a suitable place to land.\n\r",stpCh);
		global_color=0;
		return;
	}
	Flydown(stpCh);
}

void Flydown(struct char_data *stpCh)
{
int iRoom,x,y,xxx,yyy;
struct HoloSurvey *stpS,*stpSRemove;

	global_color=34;
	act("You hang on tighter...click your heels on $S sides..",FALSE,stpCh,0,stpCh->specials.stpMount,TO_CHAR);
	act("$N banks slowly and decends in small circles until you land.",FALSE,stpCh,0,stpCh->specials.stpMount,TO_CHAR);
	global_color=0;

	if(survey_list==stpCh->p->stpFlying->stpSurvey){
		survey_list=stpCh->p->stpFlying->stpSurvey->next;
		stpCh->p->stpFlying->stpSurvey->description=my_free(stpCh->p->stpFlying->stpSurvey->description);		
		stpCh->p->stpFlying->stpSurvey=my_free(stpCh->p->stpFlying->stpSurvey);
	}else{
		for(stpS=survey_list;stpS->next;stpS=stpS->next){
			if(stpS->next==stpCh->p->stpFlying->stpSurvey)
				break;
		}
		if(stpS->next){
    	 	stpSRemove=stpS->next;
        	stpS->next=stpS->next->next;
	        stpSRemove->description=my_free(stpSRemove->description);
    	    stpSRemove=my_free(stpSRemove);   
		}
	}
	if(Holo[stpCh->p->stpFlying->iX][stpCh->p->stpFlying->iY]<256)
		iMakeHoloRoom(stpCh->p->stpFlying->iX,stpCh->p->stpFlying->iY);
	iRoom=Holo[stpCh->p->stpFlying->iX][stpCh->p->stpFlying->iY];
	MOUNTMOVE=TRUE;
	char_from_room(stpCh);
	char_to_room(stpCh,iRoom);
	MOUNTMOVE=FALSE;


	x=stpCh->p->stpFlying->iX-SIGHTDIST;
	if(x<1)x=1;
	y=stpCh->p->stpFlying->iY-SIGHTDIST;
	if(y<1)y=1;
	xxx=stpCh->p->stpFlying->iX+SIGHTDIST;
	if(xxx>MAXHOLO-1)xxx=MAXHOLO-1;
	yyy=stpCh->p->stpFlying->iY+SIGHTDIST;
	if(yyy>MAXHOLO-1)yyy=MAXHOLO-1;
	HoloAct(x,y,stpCh->p->stpFlying->iX,stpCh->p->stpFlying->iY,xxx,yyy,
		"$N carrying $n, banks slowly and decends in small circles until they land.",
		TRUE,stpCh,NULL,stpCh->specials.stpMount,33,"see");

	REMOVE_BIT(stpCh->player.siMoreFlags,FLY);
	stpCh->p->stpFlying=my_free(stpCh->p->stpFlying);

}


/* this is a room we store all people that are FLYING */
void MakeFLyStoreRoom(void)
{
int x;

	for(x=0;x<MAX_ROOM;x++)
		if(!world[x])
			break;
	if(x>=MAX_ROOM)
		SUICIDE;
	CREATE(world[x],struct room_data, 1);
	number_of_rooms++;
	world[x]->number=x;
	world[x]->name=str_dup("FLYING STORAGE ROOM");
	world[x]->description=str_dup("THIS ROOM IS WHERE WE STORE PEOPLE THAT ARE FLYING.  THEY DO NOT KNOW
	OR REALIZE THEY ARE HERE.  PLEASE DO NOT TELL THEM :)
	");	
    world[x]->stpFreight=NULL; 
    world[x]->room_afs=NULL;
    world[x]->holox=0; 
    world[x]->holoy=0;
    world[x]->zone=196;
    world[x]->room_flags=0;
    world[x]->sector_type=0;
    world[x]->extra_flags=0;
	world[x]->funct=NULL;
	world[x]->contents=NULL;
	world[x]->people=NULL;
	world[x]->light=0;
	world[x]->dir_option[0]=NULL;
	world[x]->dir_option[1]=NULL;                                  	
	world[x]->dir_option[2]=NULL;                                  	
	world[x]->dir_option[3]=NULL;                                  	
	world[x]->dir_option[4]=NULL;                                  	
	world[x]->dir_option[5]=NULL;                                  	
	world[x]->ex_description=NULL;
	iFlyStoreRoom=x;
}


void StopFlying(struct char_data *stpCh)
{
struct HoloSurvey *stpS,*stpSRemove;

	if(IS_NPC(stpCh)||!IS_FLYING(stpCh))
		return;
	if(survey_list==stpCh->p->stpFlying->stpSurvey){
		survey_list=stpCh->p->stpFlying->stpSurvey->next;
		stpCh->p->stpFlying->stpSurvey->description=my_free(stpCh->p->stpFlying->stpSurvey->description);		
		stpCh->p->stpFlying->stpSurvey=my_free(stpCh->p->stpFlying->stpSurvey);
	}else{
		for(stpS=survey_list;stpS->next;stpS=stpS->next){
			if(stpS->next==stpCh->p->stpFlying->stpSurvey)
				break;
		}
		if(stpS->next){
    	 	stpSRemove=stpS->next;
        	stpS->next=stpS->next->next;
	        stpSRemove->description=my_free(stpSRemove->description);
    	    stpSRemove=my_free(stpSRemove);   
		}
	}
	REMOVE_BIT(stpCh->player.siMoreFlags,FLY);
	stpCh->p->stpFlying=my_free(stpCh->p->stpFlying);
}
