/***************************************************************************
*                MEDIEVIA CyberSpace Code and Data files                   *
*       Copyright (C) 1992, 1995 INTENSE Software(tm) and Mike Krause      *
*                          All rights reserved                             *
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
#include <math.h>

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


#define HP_WAR 1.5
#define HP_THI 1.1
#define HP_MAG .6
#define HP_CLE 1.0

#define DAM_WAR 1.3
#define DAM_MAG .5
#define DAM_CLE 1.0
#define DAM_THI 1.0

#define HMOB_MAX_TABLE	40

struct Hmob_table 
{
	sh_int hp;
	sh_int mana;
	sh_int move;
	sh_int armor;
	sbyte hitroll;
	sbyte damroll;
	byte dice[2];
	int gold;
	int exp;
	sbyte str;
	sbyte intel;
	sbyte wis;
	sbyte dex;
	sbyte con;
};
/*
 * L^2(.3HP + avg_dam + .1(200 - AC))
 */

float fMobWeights[15] = {
	0.6,			/* 0: HP			*/
	2,			/* 1: LEVEL			*/
	0.1,			/* 2: AC MOD			*/
	200,			/* 3: ARMOR TOT			*/
	5,			/* 4: HITROLL		*/
	1,			/* 5: AVERAGE DAM	*/
	1.5,			/* 6: AUTOSANC?		*/
	1.5,			/* 7: AGRESSIVE		*/
	1.2,			/* 8: FIRE SHIELD	*/
	1.01,			/* 9: SLEEPING-REST	*/
	1.3,			/*10: WIMPY		*/
 	1,			/*11: CLASS MAGE 	*/
 	1,			/*12: CLASS WARRIOR	*/
 	1,			/*13: CLASS THIEF	*/
 	1			/*14: CLASS CLERIC	*/
 	
};

const struct Hmob_table hMobTable[HMOB_MAX_TABLE+1] = 
{

/*
 *   HP,    MA,     MV,    AC,  HT, DAM, Dice[2] 
 *   GP,         EXP,      ST, IN, WS, DEX, CON
 */
 
     /* 0 */
	{ 1,	100,	100,	0,	0,	0,	1,	1,
	  1,			1,	 	1, 1, 1, 1, 1 },
     /* 1 */
	{ 18,	100,	100,	95,	0,	0,	1,	6,
	  0,			100,	 	13, 13, 13, 13, 13 },
     /* 2 */
	{ 24,	150,	100,	90,	0,	0,	1,	6,
	  0,			150,	 	13, 13, 13, 13, 13 },
     /* 3 */
	{ 32,	200,	100,	85,	0,	0,	1,	8,
	  0,			200,	 	13, 13, 13, 13, 13 },
     /* 4 */
	{ 50,	250,	100,	80,	0,	0,	1,	8,
	  0,			500,	 	13, 13, 13, 13, 13 },
     /* 5 */
	{ 100,	300,	100,	75,	0,	0,	1,	8,
	  0,			1000,	 	13, 13, 13, 13, 13 },
     /* 6 */
	{ 130,	350,	100,	70,	0,	1,	1,	8,
	  0,			3000,		13, 13, 13, 13, 13 },
     /* 7 */
	{ 150,	400,	100,	65,	0,	1,	1,	8,
	  0,			6000,	 	13, 13, 13, 13, 13 },
     /* 8 */
	{ 170,	450,	100,	60,	0,	0,	1,	10,
	  0,			8000,	 	13, 13, 13, 13, 13 },
     /* 9 */
	{ 190,	500,	100,	55,	0,	1,	1,	10,
	  0,			9000,	 	13, 13, 13, 13, 13 },
     /* 10 */
	{ 200,	550,	100,	50,	0,	1,	1,	10,
	  0,			10000,	 	14, 14, 14, 14, 14 },
     /* 11 */
	{ 250,	600,	100,	45,	0,	0,	2,	6,
	  0,			12000,	 	14, 14, 14, 14, 14 },
     /* 12 */
	{ 310,	650,	100,	40,	0,	0,	2,	6,
	  0,			15500,	 	14, 14, 14, 14, 14 },
     /* 13 */
	{ 380,	700,	100,	35,	0,	0,	2,	6,
	  0,			20000,	 	14, 14, 14, 14, 14 },
     /* 14 */
	{ 420,	750,	100,	30,	0,	0,	2,	8,
	  0,			30000,	 	14, 14, 14, 14, 14 },
     /* 15 */
	{ 480,	800,	100,	25,	0,	0,	2,	8,
	  0,			40000,	 	14, 14, 14, 14, 14 },
     /* 16 */
	{ 580,	850,	100,	20,	0,	0,	2,	8,
	  0,			60000,	 	14, 14, 14, 14, 14 },
     /* 17 */
	{ 690,	900,	100,	15,	0,	1,	2,	8,
	  0,			70000,	 	14, 14, 14, 14, 14 },
     /* 18 */
	{ 800,	950,	100,	10,	0,	0,	3,	6,
	  0,			90000,	 	14, 14, 14, 14, 14 },
     /* 19 */
	{ 900,	1000,	100,	5,	0,	0,	3,	6,
	  0,			100000,	 	14, 14, 14, 14, 14 },
     /* 20 */
	{ 1000,	1050,	100,	0,	0,	0,	3,	6,
	  0,			110000,	 	15, 15, 15, 15, 15 },
     /* 21 */
	{ 1100,	1100,	100,	-5,	0,	0,	3,	8,
	  0,			130000,	 	15, 15, 15, 15, 15 },
     /* 22 */
	{ 1210,	1150,	100,	-10,	0,	0,	4,	6,
	  0,			140000,	 	15, 15, 15, 15, 15 },
     /* 23 */
	{ 1322,	1200,	100,	-15,	0,	0,	4,	7,
	  0,			200000,	 	15, 15, 15, 15, 15 },
     /* 24 */
	{ 1440,	1250,	100,	-20,	0,	0,	4,	8,
	  0,			250000,	 	15, 15, 15, 15, 15 },
     /* 25 */
	{ 1562,	1300,	100,	-25,	0,	4,	5,	6,
	  0,			300000,	 	16, 16, 16, 16, 16 },
     /* 26 */
	{ 1690,	1350,	100,	-30,	0,	6,	5,	6,
	  0,			360000,	 	16, 16, 16, 16, 16 },
     /* 27 */
	{ 1822,	1400,	100,	-35,	0,	8,	5,	6,
	  0,			400000,	 	16, 16, 16, 16, 16 },
     /* 28 */
	{ 1960,	1450,	100,	-40,	0,	2,	5,	8,
	  0,			450000,	 	17, 17, 17, 17, 17 },
     /* 29 */
	{ 2102,	1500,	100,	-45,	0,	4,	5,	8,
	  0,			500000,	 	17, 17, 17, 17, 17 },
     /* 30 */
	{ 2250,	1550,	100,	-50,	0,	3,	5,	10,
	  0,			550000,	 	18, 18, 18, 18, 18 },
     /* 31 */
	{ 2400,	1600,	100,	-55,	0,	5,	5,	10,
	  0,			600000,	 	18, 18, 18, 18, 18 },
     /* 32 */
	{ 2560,	1650,	100,	-60,	0,	7,	5,	10,
	  0,			650000,	 	18, 18, 18, 18, 18 },
     /* 33 */
	{ 2722,	1700,	100,	-65,	5,	9,	5,	10,
	  0,			680000,	 	18, 18, 18, 18, 18 },
     /* 34 */
	{ 2890,	1750,	100,	-70,	10,	5,	5,	11,
	  0,			720000,	 	18, 18, 18, 18, 18 },
     /* 35 */
	{ 3062,	1800,	100,	-75,	20,	10,	5,	11,
	  0,			770000,	 	19, 19, 19, 19, 19 },
     /* 36 */
	{ 3888,	1900,	100,	-80,	30,	15,	5,	11,
	  0,			820000,	 	19, 19, 19, 19, 19 },
     /* 37 */
	{ 4107,	1900,	100,	-85,	40,	20,	5,	11,
	  0,			900000,	 	20, 20, 20, 20, 20 },
     /* 38 */
	{ 4332,	2000,	100,	-90,	50,	20,	5,	12,
	  0,			950000,	 	21, 21, 21, 21, 21 },
     /* 39 */
	{ 5000,	2100,	100,	-95,	60,	20,	6,	11,
	  0,			1000000,	 	22, 22, 22, 22, 22 },
     /* 40 */
	{ 6100,	2300,	100,	-100,	70,	30,	6,	12,
	  0,			1500000,	 	23, 23, 23, 23, 23 }

};
	


int iMobWeigh(struct char_data *mob)
{
int iBase=0, iAvDam, iAvDr; 
int iLev, iHp, iArm;
	
	/* Figuring out Base Stuff */

	iHp = pow( (double) mob->points.max_hit, (double) fMobWeights[0]);

	iArm = (fMobWeights[3] - mob->points.armor) * fMobWeights[2];
		
	iLev = pow( (double) GET_LEVEL(mob), (double) fMobWeights[1]);

	iAvDam = ( mob->specials.damnodice + (mob->specials.damnodice*mob->specials.damsizedice) ) / 2;
		
	switch(GET_CLASS(mob)) {
		case CLASS_WARRIOR:
			iAvDr = mob->points.damroll;
			break;
		case CLASS_THIEF:
			iAvDr = mob->points.damroll * .75;
			break;
		case CLASS_CLERIC:
			iAvDr = mob->points.damroll / 2;
			break;
		case CLASS_MAGIC_USER:
			iAvDr = mob->points.damroll / 2;
			break;
		default:
		    iAvDr = mob->points.damroll / 2;
			break;
	}
	
	/* Calculate the final damange amounts and then multiply by weight */
	
	iAvDam += iAvDr;
	
	iAvDam *= fMobWeights[5];

	iBase += iLev * ( iHp + iAvDam  );

	
	switch(GET_CLASS(mob)) {
		case CLASS_WARRIOR:
			iBase *= fMobWeights[12];
			break;
		case CLASS_CLERIC:
			iBase *= fMobWeights[14];
			break;
		case CLASS_MAGIC_USER:
			iBase *= fMobWeights[11];
			break;
		case CLASS_THIEF:
			iBase *= fMobWeights[13];
			break;
		default:
			break;
	}

	/*
	 *         Now onto fireshield | sanc affects | etc 
	 */
	
	if(IS_SET(mob->specials.affected_by, AFF_SANCTUARY))
		iBase *= fMobWeights[6];
	if(IS_SET(mob->specials.affected_by, AFF_FIRESHIELD))
		iBase *= fMobWeights[8];	
	if(IS_SET(mob->specials.act, ACT_AGGRESSIVE))
		iBase *= fMobWeights[7];
	if(IS_SET(mob->specials.act, ACT_WIMPY))
		iBase *= fMobWeights[11];
		
	if(iBase >= 0)
		return(iBase);
	else
		return(0);
		
}


void do_Mobexp(struct char_data *ch, char *argument, int cmd)
{
	char arg1[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	char arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
	int loop=0, value;
	float value2;
	struct char_data *mob;
	
	char *wt_types[] =
	{
		"(hp)          x:",
		"(level)       y:",
		"(AC mod)      z:",
		"(AC sub)      a:",
		"(NU)    Hitroll:",
		" Average Damage:",
		"(M)   Sanctuary:",
		"(M)  Aggressive:",
		"(M) Fire Shield:",
		"(M)(NU)Sleeping:",
		"(M)       Wimpy:",
		"(M)  Class Mage:",
		"(M) Class Thief:",
		"(M)   Class War:",
		"(M)Class Cleric:"
	};		
	
	if(ch == NULL || IS_NPC(ch)) return;
	
	*buf = '\0';
	
	if(!*argument) {
		send_to_char("MOB EXPERIENCE MODIFIERS/WEIGHTS\r\n",ch);
		send_to_char("EQ: lev^y( hp^x + AvDam )\r\n",ch);
		for(loop = 0; loop <= 14; loop++) 
		{
			sprintf(buf, "[%s%2d%s] %s %s%f%s\r\n",
				 GRN(ch), loop, NRM(ch), wt_types[loop], RED(ch),
				 fMobWeights[loop], NRM(ch));
				 send_to_char(buf,ch);
		} 
		return;
	}
	
	argument = one_argument(argument,arg1);  
	argument = one_argument(argument,arg2);
	one_argument(argument,arg3);
	
	
	if(arg1[0] != 's' && arg1[0] != 'm') {
		send_to_char("Usage: mobexp (m||s) (name||value)\r\n",ch);
		send_to_char("THIS CAN BE DANGEROUS! BE VERY CAREFULL!\r\n",ch);	
		return;
	}
	
	if(arg1[0] == 'm') {
		if(!*arg2) {
			send_to_char("You must specify a mob name to test.\r\n",ch);
			return;
		}
		
		mob = get_char_vis(ch,arg2);
		
		if(mob) {
			value = iMobWeigh(mob);
			sprintf(buf,"%s is worth: %d exps.", GET_NAME(mob), value);
			send_to_char(buf,ch);
			return;
		} else {
			send_to_char("No such creature. \r\n",ch);
			return;
		}
	}
	
	if(arg1[0] == 's') {
		if(!*arg2 || !*arg3) {
			send_to_char("You must specify a value to set..\r\n",ch);
			return;
		}
		value = atoi(arg2);
		value2 = atof(arg3);
		
		if(value > 14 || value < 0) {
			send_to_char("Invalid Modifier.\r\n",ch);
			return;
		}
		fMobWeights[value] = value2;
		send_to_char("MOB EXP: Weights Updated.\r\n",ch);
		
		return;
	}
	
}
			
	
	
void SetMobDificulty(struct char_data *mob, int level)
{
sh_int iHp=0;

	if(!mob) {
		log_hd("## Null mob passed to SetMobDifculty");
		return;
	}

	if( !IS_NPC(mob) ) 
		return;
	
	if(level > 40) {
		log_hd("## mob LEVEL too HIGH in Setmobdificulty");
		level= 40;
	}
	
	if(level < 1) {
		log_hd("## Mob LEVEL to LOW in Setmobdificulty");
		level = 1;  
	}
	
	iHp = hMobTable[level].hp + ( (hMobTable[level].hp / 100) * number(-10,10) );
	
    
	switch( GET_CLASS(mob) ) {
		case CLASS_WARRIOR:
			mob->points.max_hit = iHp * HP_WAR;
			mob->points.damroll = hMobTable[level].damroll * DAM_WAR;
			break;
		case CLASS_CLERIC:
			mob->points.max_hit = iHp * HP_CLE;
			mob->points.damroll = hMobTable[level].damroll * DAM_CLE;
			break;
		case CLASS_MAGIC_USER:
			mob->points.max_hit = iHp * HP_MAG;
			mob->points.damroll = hMobTable[level].damroll * DAM_MAG;
			break;
		case CLASS_THIEF:
			mob->points.max_hit = iHp * HP_THI;
			mob->points.damroll = hMobTable[level].damroll * DAM_THI;
			break;
		default:
			mob->points.max_hit = iHp;
			mob->points.damroll = hMobTable[level].damroll;
			break;
	}
	
	mob->points.max_mana = hMobTable[level].mana;
	mob->points.max_move = hMobTable[level].move;
	                        
	mob->abilities.str   = hMobTable[level].str;
	mob->abilities.dex   = hMobTable[level].dex;
	mob->abilities.con   = hMobTable[level].con;
	mob->abilities.wis   = hMobTable[level].wis;
	mob->abilities.intel = hMobTable[level].intel;	                                        
					
	mob->points.hitroll  = hMobTable[level].hitroll;
	mob->specials.damnodice = hMobTable[level].dice[0];
	mob->specials.damsizedice = hMobTable[level].dice[1];
	mob->points.armor = hMobTable[level].armor;
	
	mob->points.gold = hMobTable[level].gold + ( (hMobTable[level].gold / 100) * number(-20,20) );
	mob->points.exp = hMobTable[level].exp + ( (hMobTable[level].exp / 100) * number(-20,20) );	

	GET_LEVEL(mob) = level;
	
	GET_HIT(mob) = GET_MAX_HIT(mob);
	GET_MANA(mob) = GET_MAX_MANA(mob);
	GET_MOVE(mob) = GET_MAX_MOVE(mob);
	
	return;

}
