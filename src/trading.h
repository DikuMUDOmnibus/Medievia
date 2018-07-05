/***************************************************************************
*					 MEDIEVIA CyberSpace Code and Data files		       *
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

struct TRADABLE{
	int iSize;
	int iWeight;
	char *szpName;
	char *szpDescription;
	char *szpUnitName;
	int iOrigenX;
	int iOrigenY;
	int iNumInGame;
	int iWorth;
};

struct FREIGHT{
	int iType;
	long int liStatus;
	int iLocationX;
	int iLocationY;
	int iLocationRoom;
	int iCurrentWeight;
	int iCurrentSize;
	struct char_data *stpCh;
	char szOwnersName[14];
	int iaCargo[100];
	struct FREIGHT *stpNext;
	struct FREIGHT *stpNextInRoom;
};

struct FREIGHTTYPE{
	int iMaxSize;
	int iMaxWeight;
	char *szpName;
	char *szpShortDescription;
	char *szpDescription;
	int iPrice;
	int iTypeBits;
	struct char_data *stpMob;
};

struct TRADESHOP{
	char *szpName;
	char *szpDescription;
	int iX;
	int iY;
	int iaTrading[5];
	int iaStock[100];
};

#define MAXTRADINGSHOPS 25

/*Freight Statuses */
#define HIDDEN		1
#define TEHERED		2
#define HITCHED		4
#define PROTECTED	8

/*Freight Types*/
#define WAGONTYPE	1
#define MULETYPE	2
#define HORSETYPE	4
