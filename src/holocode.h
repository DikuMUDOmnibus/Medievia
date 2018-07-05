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

#define MAXHOLO		2000

#define HOLORX(num)       (world[(num)]->holox ? world[(num)]->holox : zone_table[world[(num)]->zone].siX)
#define HOLORY(num)       (world[(num)]->holoy ? world[(num)]->holoy : zone_table[world[(num)]->zone].siY)

#define HOLOX(ch)       (world[(ch)->in_room]->holox ? world[(ch)->in_room]->holox : zone_table[world[(ch)->in_room]->zone].siX)
#define HOLOY(ch)       (world[(ch)->in_room]->holoy ? world[(ch)->in_room]->holoy : zone_table[world[(ch)->in_room]->zone].siY)

#define FLAG_BEACON		1
#define FLAG_NOSEEDARK		2
#define FLAG_GLOW		4

void HoloAct(int x,int y,int hx, int hy,int xxx,int yyy,char *szpText, bool bInvis, 
            struct char_data *stpCh, struct obj_data *stpObject, 
            struct char_data *stpVict, int iColor, char *szpSense);


struct HoloSurvey{
	short int iX;
	short int iY;
	unsigned short int dist;
	char *description;
	sh_int siFlags; /*For Future Expansion */
	struct HoloSurvey *next;
};	

struct HOLOLINKS{
    unsigned short int iX;
    unsigned short int iY;
    char cDir;
    int iRoom;
};


struct HOLOROOMS{
	int number;
	unsigned char sector_type;
	int extra_flags;
	int hurt_num_die;
	int hurt_size_die;
	int hurt_base;
	unsigned char class_restriction;
	unsigned short int level_restriction;	
	unsigned short int align_restriction;	
	int mount_restriction;
	short int move_mod;
	int pressure_mod;
	int temperature_mod;
	char *name;
	char *description[10];
	short int room_flags;
	char cMapChar;
	short int siMapColor;
};
