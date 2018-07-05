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


#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "handler.h"
#include "interp.h"
#include "db.h"

/* uses */
extern char global_color;
extern struct time_info_data time_info;
extern struct weather_data weather_info;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
/* In this part. */

void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);

char lightening=0;
/* Here comes the code */

void weather_and_time(int mode)
{
struct descriptor_data *i=NULL;
int r=0;
    another_hour(mode);
    if(mode)
	weather_change();
    if(lightening)global_color=33;
    if(lightening)
    for(i=descriptor_list;i;i=i->next)
	if((!i->connected)
	   &&!IS_SET(ORIGINAL(i->character)->specials.plr_flags, PLR_WRITING))
	    if(OUTSIDE(i->character)&&AWAKE(i->character)){
		r=number(1,100);	
		switch(r){
		    case 1:
			r=number(1,20);
			if(r!=1)break;
			send_to_char("BANG-ZAP!	LIGHTNING STRIKES YOU!\n\r", i->character);
			GET_HIT(i->character)=number(1,10);
			act("ZAP! Lightning strikes $n!\n\r",TRUE, i->character,0, 0, TO_ROOM);
			global_color=0;
			return;
			break;
		    case 2:
			send_to_char("BANG!   You feel your hairs stand on end lightning just misses you!\n\r", i->character);
			break;
		    case 3:
			send_to_char("BANG!  Lightning strikes right next to you!\n\r", i->character);
			break;
		    case 4:
			send_to_char("BANG!  Whew THAT WAS CLOSE! \n\rYou are momentarily blinded by lightning that lands 20 feet ahead!\n\r", i->character);
			break;
		    case 5:
			send_to_char("BANG!  You feel the adrenaline excite your body as lightning lands 30 feet to your side.\n\r", i->character);
			break;
		    case 6:
			send_to_char("BABOOM!  You try and clear the after image of lightning from your eyes.\n\r", i->character);
			break;
		    case 7:
			send_to_char("BABOOM!  You pray to the gods as lightning lands 40 feet behind you. \n\r", i->character);
			break;
		    case 8:
			send_to_char("BOOOOM!  Lightning hits very nearby.\n\r", i->character);
			break;
		    case 9:
			send_to_char("BOOOOM!  Lightning hits in the distance.\n\r", i->character);
			break;
		    case 10:
			send_to_char("BOOOOM!  Lightning hits in the far distance.\n\r", i->character);
			break;
		    case 11:
			send_to_char("BOOOOM!  You hear the lightning hit in some nearby land.\n\r", i->character);
			break;
		    default:
			send_to_char("You see the flash of distant lightning illuminate the land.\n\r", i->character);
			break;
		}
	    }
    global_color=0;
}



void another_hour(int mode)
{
FILE *fh;

    time_info.hours++;
	if((time_info.hours%20)==0){
    	if((fh=med_open("../lib/time.dat", "w"))!=NULL){
           	fwrite(&time_info,sizeof(time_info), 1, fh);
            med_close(fh);
        }
	}
    global_color=34;
    if (mode) {
	switch (time_info.hours) {
	    case 5 :
	    {
		weather_info.sunlight = SUN_LIGHT;
		send_to_outdoor("The day has begun.\n\r");
		break;  
	    }
	    case 6 :
	    {
		weather_info.sunlight = SUN_RISE;
		send_to_outdoor("The sun's light makes its first appearance from the southern horizon.\n\r");
		break;
	    }
	    case 21 :
	    {
		weather_info.sunlight = SUN_SET;
		send_to_outdoor(
		"The sun slowly disappears from view over the northern horizon.\n\r");
		break;
	    }
	    case 22 :
	    {
		weather_info.sunlight = SUN_DARK;
		send_to_outdoor("The night has begun, and darkness fills the realm.\n\r");
		break;
	    }
	    default : break;
	}
    }
    global_color=0;
    if (time_info.hours > 23)  
    {
	time_info.hours -= 24;
	time_info.day++;

	if (time_info.day>34)
	{
	    time_info.day = 0;
	    time_info.month++;

	    if(time_info.month>16)
	    {
		time_info.month = 0;
		time_info.year++;
	    }
	}
    }
}

void weather_change(void)
{
    int diff, change;
    if((time_info.month>=9)&&(time_info.month<=16))
	diff=(weather_info.pressure>985 ? -2 : 2);
    else
	diff=(weather_info.pressure>1015? -2 : 2);

    weather_info.change += (dice(1,4)*diff+dice(2,6)-dice(2,6));

    weather_info.change = MIN(weather_info.change,12);
    weather_info.change = MAX(weather_info.change,-12);

    weather_info.pressure += weather_info.change;

    weather_info.pressure = MIN(weather_info.pressure,1040);
    weather_info.pressure = MAX(weather_info.pressure,960);

    change = 0;

    switch(weather_info.sky){
	case SKY_CLOUDLESS :
	{
	    if (weather_info.pressure<990)
		change = 1;
	    else if (weather_info.pressure<1010)
		if(dice(1,4)==1)
		    change = 1;
	    break;
	}
	case SKY_CLOUDY :
	{
	    if (weather_info.pressure<970)
		change = 2;
	    else if (weather_info.pressure<990)
		if(dice(1,4)==1)
		    change = 2;
		else
		    change = 0;
	    else if (weather_info.pressure>1030)
		if(dice(1,4)==1)
		    change = 3;

	    break;
	}
	case SKY_RAINING :
	{
	    if (weather_info.pressure<970)
		if(dice(1,4)==1)
		    change = 4;
		else
		    change = 0;
	    else if (weather_info.pressure>1030)
		    change = 5;
	    else if (weather_info.pressure>1010)
		if(dice(1,4)==1)
		    change = 5;

	    break;
	}
	case SKY_LIGHTNING :
	{
	    if (weather_info.pressure>1010)
		    change = 6;
	    else if (weather_info.pressure>990)
		if(dice(1,4)==1)
		    change = 6;

	    break;
	}
	default : 
	{
	    change = 0;
	    weather_info.sky=SKY_CLOUDLESS;
	    break;
	}
    }
    global_color=34;
    switch(change){
	case 0 : break;
	case 1 :
	{
	    send_to_outdoor(
	    "The sky is getting cloudy.\n\r");
	    weather_info.sky=SKY_CLOUDY;
	    break;
	}
	case 2 :
	{
	    send_to_outdoor(
	    "It starts to rain.\n\r");
	    weather_info.sky=SKY_RAINING;
	    break;
	}
	case 3 :
	{
	    send_to_outdoor(
	    "The clouds disappear.\n\r");
	    weather_info.sky=SKY_CLOUDLESS;
	    break;
	}
	case 4 :
	{
	    send_to_outdoor(
	    "Lightning starts to show in the sky.\n\r");
	    weather_info.sky=SKY_LIGHTNING;
	    lightening=1;
	    break;
	}
	case 5 :
	{
	    send_to_outdoor(
	    "The rain stopped.\n\r");
	    weather_info.sky=SKY_CLOUDY;
	    break;
	}
	case 6 :
	{
	    send_to_outdoor(
	    "The lightning has stopped.\n\r");
	    weather_info.sky=SKY_RAINING;
	    lightening=0;
	    break;
	}
	default : break;
    }
    global_color=0;
}
