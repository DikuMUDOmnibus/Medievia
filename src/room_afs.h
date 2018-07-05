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

#define RA_BLOOD  	    1	/* pools of blood messages			*/
#define RA_SHIELD	    2	/* mage shield room spell			*/
#define RA_ASHES	    3	/* Fireball (ashes) message			*/
#define RA_SLAY		    4	/* God slayed someone message		*/
#define RA_ICE		    5	/* Mage ICE spell 					*/
#define RA_EARTHQUAKE	6	/* earthquake was here message		*/
#define RA_MESSAGE	    7	/* god made a message here			*/
#define RA_TRAP	    	8   /* Traps thiefs lay					*/
#define RA_MAP		    9   /* Used for catacomb mapping		*/
#define RA_METHANE		10  /* Used for the gas cloud 			*/
#define RA_SCENT		11  /* Mobile tracking */
#define RA_CAMP         12  /* A campsite is made here         	*/

struct room_affect{
    char type;
    char timer; /* each tick is about 30 real seconds on average */
    int value;
    char *text;
    int  room;
    struct char_data *ch;
    struct room_affect *next;
};
