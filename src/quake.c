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
#include <assert.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "handler.h"
#include "interp.h"
#include "db.h"
#include "spells.h"

extern struct room_data *world[MAX_ROOM];  
extern struct char_data *character_list;  
void fall(struct char_data *ch);
void Earthquake(void);
void FakeEarthquake(void);
void Shake(void);
void die_formation(struct char_data *ch);
extern bool is_formed(struct char_data *ch);
extern char global_color;
extern struct index_data mob_index_array[MAX_MOB]; 
extern struct index_data *mob_index;  

char *szaIndoors[]= {
    "The floor Vibrates slightly under your feet..\n\r",
    "The floor Trembles beneath your feet..\n\r",
    "You hear a low rumbling sound as the floor shudders\n\r",
    "The ground Pulsates and Shakes.. .. .\n\r",
    "LOUD Rumbling Noise hurts your ears and the floor Rocks to and fro..\n\r",
    "The floor CONVULSES Mightily..you are rocked around violently..\n\r",
    "The Whole Area RUMBLES with anger..SHAKES with POWER.. .\n\r",
    "The ground HEAVES AND SHAKES...the sheer POWER R-I-P-S through the area..\n\r",
    "Your Vision is BLURRED..everything BANGS AND VIBRATES..so FAST\n\r",
    "Everything M O V E S UP and DOWN, LEFT and RIGHT..YOU SCREAM!!  \n\r"
};
char *szaOutdoors[]= {
    "The area Vibrates...slightly...under your feet..\n\r",
    "The ground trembles beneath you...a low moan rolls by...\n\r",
    "You hear a low rumbling as the earth shudders..\n\r",
    "The ground Pulsates and Shakes...   .    .\n\r",
    "LOUD rumbling hurts your ears..the ground rocks to and fro..\n\r",
    "The ground CONVULSES mightily..you are rocked around violently..\n\r",
    "The Whole area RUMBLES with Anger...SHAKES with POWER.. .\n\r",
    "The Earth HEAVES AND SHAKES..the Sheer POWER R-I-P-S through the area..\n\r",
    "Your Vision is BLURRED..everything BANGS AND VIBRATES.. so FAST!\n\r",
    "The EARTH COMES ALIVE AGAIN...SHAKING in ALL directions!\n\r"
};
char *szaCatacombs[]= {
    "The catacombs Vibrates...slightly under your feet...\n\r",
    "The combs tremble...a low moan rolls through the area..\n\r",
    "You hear a LOW Rumble...Dust is everywhere...the area shudders..\n\r",
    "The ground Pulsates and Shakes...\n\r",
    "LOUD rumbling hurts your ears..the combs rock to and fro\n\r",
    "The Combs CONVULSE Mightily..you are Rocked around Violently..\n\r",
    "The Whole area RUMBLES with Anger...SHAKES with POWER...\n\r",
    "The Combs HEAVES AND SHAKE..the Sheer POWER R-I-P-S through the area..\n\r",
    "Your vision is BLURRED..Everything BANGS AND VIBRATES..so FAST!\n\r",
    "The COMBS COME ALIVE AGAIN..SHAKING in ALL directions!\n\r"
};
char *szaOcean[]= {
    "s0\n\r",
    "s1\n\r",
    "s2\n\r",
    "s3\n\r",
    "s4\n\r",
    "s5\n\r",
    "s6\n\r",
    "s7\n\r",
    "s8\n\r",
    "s9\n\r"
};

struct EARTHQUAKE *stpEarthquake;

void StartEarthquake(void) 
{
 
    if(number(1,100)<35) {
		Earthquake();
    }else {
		FakeEarthquake();
    }
}

void FakeEarthquake(void)
{
    CREATE(stpEarthquake,struct EARTHQUAKE, 1);
    if(!stpEarthquake) {
		log_hd("### could not create Earthquake");
		return;
    }
    stpEarthquake->bRealQuake=FALSE;
    stpEarthquake->iLengthInSeconds=number(10,30);
    stpEarthquake->iRictor=number(20,40);
    time(&stpEarthquake->time_tStartTime);
    stpEarthquake->iNumShakes=0;
    Shake();
}

void Earthquake(void)
{
int t;
    
    CREATE(stpEarthquake,struct EARTHQUAKE, 1);
    if(!stpEarthquake) {
	log_hd("### could not create Earthquake");
	return;
    }
    stpEarthquake->bRealQuake=TRUE;
    t=number(50,85);
    stpEarthquake->iRictor=number(5,30)+t;
    if(stpEarthquake->iRictor>100)
    	stpEarthquake->iRictor=100;
    stpEarthquake->iLengthInSeconds=t;
    time(&stpEarthquake->time_tStartTime);
    stpEarthquake->iNumShakes=0;
    Shake();
}

void Shake(void)
{
time_t t;
float iNow,iEnd,iHowFarDone,iMessage,iNextShake,iViolence,iToLoose,iAveShakes;
struct char_data *stpCh;
int SCALE[]= {0,1,2,3,4,5,6,7,8,9,10,9,8,7,6,5,4,3,2,1};
int SECONDS[]={9,9,8,7,6,5,4,3,2,1,1,1,2,3,4,5,6,7,8,9};
float fThisRound;
    
    if(!stpEarthquake)return;
    time(&t);
    if(t>(stpEarthquake->time_tStartTime+stpEarthquake->iLengthInSeconds)) {
		stpEarthquake=my_free(stpEarthquake);
		for(stpCh=character_list;stpCh;stpCh=stpCh->next) {
			stpCh->specials.iEarthquakeHits=0;
	    }
		return;
    }
    
    iNow=t-stpEarthquake->time_tStartTime;
    iEnd=stpEarthquake->iLengthInSeconds;
    iHowFarDone=(20*(iNow/iEnd));
    if(iHowFarDone<0)
    	iHowFarDone=0;
    if(iHowFarDone>19)
    	iHowFarDone=19;
    iNextShake=SECONDS[(int)(iHowFarDone)];
    iHowFarDone=SCALE[(int)(iHowFarDone)];
    iViolence=(int)(iHowFarDone*(stpEarthquake->iRictor/100));
    iMessage=iViolence+number(-2,2);
    if(iMessage<0)
    	iMessage=0;
    if(iMessage>9)
    	iMessage=9;
    iNextShake+=number(-1,1);
    if(iNextShake<1)
    	iNextShake=1;
    if(iNextShake>9)
    	iNextShake=9;
	stpEarthquake->time_tNextShake=t+(int)(iNextShake);
	iAveShakes=(iEnd/5);	
	if(iAveShakes>3)iAveShakes-=2;
	fThisRound=(iEnd*(stpEarthquake->iRictor/100));
	fThisRound/=iAveShakes;
    fThisRound/=100;
	global_color=33;
    for(stpCh=character_list;stpCh;stpCh=stpCh->next) {
		if(!IS_NPC(stpCh)&&IS_FLYING(stpCh))
			continue;
		/* So game dosn't crash on FreightSocialMobs */
		if(stpCh->in_room == -1) continue;
		if(stpCh->desc) {
			if(stpCh->desc){
		    	if(GET_ZONE(stpCh)==198)
		    		send_to_char(szaCatacombs[(int)(iMessage)],stpCh);
			    else if(OUTSIDE(stpCh))
			    	send_to_char(szaOutdoors[(int)(iMessage)],stpCh);
		    	else
			    	send_to_char(szaIndoors[(int)(iMessage)],stpCh);
			}
    	}
		if(stpEarthquake->bRealQuake){
			if(!stpEarthquake->iNumShakes)
				stpCh->specials.iEarthquakeHits=(double)(GET_HIT(stpCh));
			if((((stpEarthquake->iNumShakes>2)&&(GET_POS(stpCh)==POSITION_FIGHTING))||(stpCh->specials.fighting)))
			    if(stpCh->specials.fighting)	
				stop_fighting(stpCh);
			if(stpEarthquake->iNumShakes>4&&is_formed(stpCh))
				die_formation(stpCh);
			iToLoose=(int)(stpCh->specials.iEarthquakeHits*fThisRound);
			if(GET_POS(stpCh)<=POSITION_SITTING)
				iToLoose/=2;
			if(!IS_NPC(stpCh)&&IS_AFFECTED(stpCh,AFF_FLYING))
				iToLoose/=2;
			if(!IS_NPC(stpCh)&&OUTSIDE(stpCh))
				iToLoose/=2;
			if(IS_MOB(stpCh)&&mob_index[stpCh->nr].func==shop_keeper)
				continue;
			if(IS_NPC(stpCh))
				iToLoose-=iToLoose/25;
			GET_HIT(stpCh)-=iToLoose;
			if (EXIT(stpCh,5))
				fall(stpCh);
			if(IS_NPC(stpCh)){
				if(GET_HIT(stpCh)<(GET_MAX_HIT(stpCh)/3)){
					GET_HIT(stpCh)=GET_MAX_HIT(stpCh)/3;
					if(stpCh->specials.iEarthquakeHits>GET_HIT(stpCh))
						GET_HIT(stpCh)=stpCh->specials.iEarthquakeHits;
				}				
			}else{
				if(GET_HIT(stpCh)<20)
					GET_HIT(stpCh)=number(15,25);
			}
		}
    }
	stpEarthquake->iNumShakes++;
	global_color=0;
}

void do_quake(struct char_data *ch, char *argument, int cmd)
{
char buf[MAX_INPUT_LENGTH];

	if(stpEarthquake){
		send_to_char("One earthquake at a time man!\n\r",ch);
		return;
	}
	one_argument(argument,buf);
	if(!buf[0]){
	    send_to_char("Sytax->quake R  for real quake and ->quake F for fake\n\r",ch);
	    send_to_char("VRYCE DOES  (NOT) WANT THIS COMMAND ABUSED, it is logged\n\r",ch);
		return;
	}
	if(buf[0]=='R'||buf[0]=='r'){
		Earthquake();
	}else if(buf[0]=='F'||buf[0]=='f'){
		FakeEarthquake();
	}else{
	    send_to_char("Sytax->quake R  for real quake and ->quake F for fake\n\r",ch);
	    send_to_char("VRYCE DOES  (NOT) WANT THIS COMMAND ABUSED, it is logged\n\r",ch);
	}	
}
