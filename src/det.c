/* Seconds in week 604800 */
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
#include "interp.h"

/*                  /External variables and functions\                    */
/*------------------------------------------------------------------------*/
extern int port;
extern struct room_data *world[MAX_ROOM];  
extern struct char_data *mobs[MAX_MOB];  
extern struct obj_data *objs[MAX_OBJ];  
extern int top_of_world;  
extern struct obj_data *object_list;  
extern struct char_data *character_list;  
extern struct index_data obj_index_array[MAX_OBJ];  
extern struct index_data *obj_index;  
extern struct descriptor_data *descriptor_list;  
extern int dice(int number, int size);  
extern int number(int from, int to); 
extern int VERSIONNUMBER;
extern int weigh(struct obj_data *obj);
void det(struct obj_data *obj, int percent);

const char *szaDetStatusMessages[] = {
"The object appears to be in perfect pristine condition.\r\n",
"The object appears to be in excellent condition.\r\n",
"The object appears to be in good condition.\r\n",
"The object appears to be in fair condition.\r\n",
"The object is in fair condition but has some scratches.\r\n",
"The object clearly shows major signs of wear and tear.\r\n",
"The object is visibly worn down with major wear.\r\n",
"The life of this object is clearly comming to an end soon.\r\n",
"The object looks as if it will fall apart any day now.\r\n",
"The object is visibly crumbleing and decaying....\r\n ",
"ERROR: REPORT TO A GOD IMMEDIATLY.. DET STATUS IN IDENTIFY..\r\n"
};


struct obj_data *deteriorate(struct obj_data *obj)
{
int iTimesToDet=0, iTime, i;

	if(obj->iDetLife <= 0)	
		return(obj);
	
	if(!obj->iBornDate)
		return(obj);

	iTime = (int) time(NULL);
	
	if(iTime > (obj->iBornDate+obj->iDetLife))
	{
		if(obj->iLastDet > obj->iBornDate) {
			iTimesToDet = (iTime - obj->iLastDet) / 1209600;
			for(i = 0;i < iTimesToDet;i++)
				det(obj,13);
		} else {
			/* FIRST TIME OBJECT IS BEING DET'D */
			/* If more than one week has passed since it crossed the DETLine!*/
			iTimesToDet = (iTime - (obj->iBornDate+obj->iDetLife))/1209600;
			if(iTimesToDet>0) {
				for(i = 0;i <= iTimesToDet;i++)
					det(obj,13);
			} else det(obj,13);		
		}
	}
	return(obj);
		
}


void det(struct obj_data *obj, int percent)
{
float fMulti;
int i;

	if(!obj)
		return;
		
	fMulti = (100-(float)percent)/100;
	
	for (i=0;i<MAX_OBJ_AFFECT;i++) 	
		obj->affected[i].modifier *= fMulti;
		
	if(obj->obj_flags.type_flag == ITEM_ARMOR)
		obj->obj_flags.value[0] *= fMulti;
		
	if(obj->obj_flags.type_flag == ITEM_WEAPON)
	{
		obj->obj_flags.value[1] *= fMulti;
		obj->obj_flags.value[2] *= fMulti;
	}
	obj->obj_flags.cost *= fMulti;
	
	obj->iLastDet = time(NULL);
	obj->iValueWeight = weigh(obj);
	obj->iWeightVersion = VERSIONNUMBER;
	
}
