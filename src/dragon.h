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

#define PEOPLEPERDRAGON		1
#define MAXDRAGONS			50
#define MAXDRAGONTYPES		50
#define SIGHTDIST			60
#define PULSEBEFOREHUNT		4*90
#define PULSEBEFORESTOPHUNT 4*60*2
#define FLYTIMEPULSE		4*60*7

struct DRAGONSTRUCT{
	struct char_data *stpDragon;
	int iPulse;
	struct HoloSurvey *stpSurvey;
	int iStatus;
	int iX;
	int iY;
	int iRoom;
	struct char_data *stpCaller;
};

#define CIRCLING	0	/* An evil dragon looking for prey				*/
#define HUNTING		1	/* An evil dragon on ground tracking his prey	*/
#define FIGHTING	2	/* An evil dragon fighting his prey				*/
#define RESPONDING	4	/* A good dragon in air heading towards caller	*/
#define DEFENDING	8	/* A good dragon fighting an evil dragon		*/
#define FLYING		16	/* A good dragon flying carrying a player		*/
